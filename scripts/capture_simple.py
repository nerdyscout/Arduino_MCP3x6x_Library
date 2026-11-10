#!/usr/bin/env python3
"""Simple SPI capture from Rigol DS1104Z via USBTMC."""
import pyvisa
import time
import serial

SCOPE_RESOURCE = "USB::0x1AB1::0x04CE::DS1ZA233407782::INSTR"
CH_CS = 1   # CH1 = CS   (D10)
CH_MISO = 2   # CH2 = MISO (D12)
CH_CLK = 3    # CH3 = CLK  (D13)
CH_MOSI = 4   # CH4 = MOSI (D11)
THRESHOLD_V = 2.5

def read_waveform_raw(inst, channel):
    """Read waveform data from a channel via SCPI."""
    inst.write(f":WAV:SOURce CHANnel{channel}")
    inst.write(":WAV:FORMat BYTE")
    inst.write(":WAV:POINts:MODE RAW")
    pre = inst.query(":WAV:PREamble?").strip()
    parts = pre.split(",")
    xinc = float(parts[4])
    xo = float(parts[5])
    xref = int(parts[6])
    yinc = float(parts[7])
    yo = float(parts[8])
    yref = int(parts[9])

    inst.write(f":WAV:SOURce CHANnel{channel}")
    inst.write(":WAV:FORMat BYTE")
    raw = inst.read_raw()
    time.sleep(0.1)

    if len(raw) == 0:
        return [], pre

    if raw[0:1] == b"#":
        ndig = int(raw[1:2])
        length = int(raw[2:2 + ndig])
        data = raw[2 + ndig:2 + ndig + length]
    else:
        data = raw

    samples = []
    for i, b in enumerate(data):
        t = xo + (i - xref) * xinc
        v = yo + (b - yref) * yinc
        samples.append((t, v))

    return samples, pre

def to_logic(samples, threshold=2.5):
    return [1 if v >= threshold else 0 for _, v in samples]

def bits_to_bytes(bits):
    out = []
    for j in range(0, len(bits), 8):
        byte = 0
        for k in range(8):
            if j + k < len(bits):
                byte = (byte << 1) | bits[j + k]
        out.append(byte)
    return out

def decode_spi(cs_logic, clk_logic, mosi_logic, miso_logic, times):
    transactions = []
    i = 0
    n = len(cs_logic)

    while i < n - 1:
        while i < n - 1 and not (cs_logic[i] == 1 and cs_logic[i + 1] == 0):
            i += 1
        if i >= n - 1:
            break

        cs_start = i
        i += 1
        prev_clk = clk_logic[i] if i < n else 0

        bits_mosi = []
        bits_miso = []

        while i < n - 1 and cs_logic[i] == 0:
            if clk_logic[i] == 1 and prev_clk == 0:
                bits_mosi.append(mosi_logic[i])
                bits_miso.append(miso_logic[i])
            prev_clk = clk_logic[i]
            i += 1

        if len(bits_mosi) >= 8:
            mosi_bytes = bits_to_bytes(bits_mosi)
            miso_bytes = bits_to_bytes(bits_miso)
            transactions.append({
                "t_start_us": round(cs_start * xinc * 1e6, 2),
                "mosi": [f"0x{b:02X}" for b in mosi_bytes],
                "miso": [f"0x{b:02X}" for b in miso_bytes],
            })

        while i < n - 1 and not (cs_logic[i] == 0 and cs_logic[i + 1] == 1):
            i += 1

    return transactions

def main():
    rm = pyvisa.ResourceManager("@py")
    inst = rm.open_resource(SCOPE_RESOURCE, timeout=10000)
    inst.write_termination = "\n"
    inst.read_termination = "\n"

    print(f"Connected to {inst.query('*IDN?').strip()}")

    # Reset scope
    inst.write("*RST")
    inst.write("*CLS")
    time.sleep(1)

    # Configure channels
    for ch in range(1, 5):
        inst.write(f":CHANnel{ch}:DISPlay ON")
        inst.write(f":CHANnel{ch}:COUPling DCLimit")
        inst.write(f":CHANnel{ch}:SCALe 5.0")
        inst.write(f":CHANnel{ch}:OFFSet 0")
    time.sleep(0.5)

    # Trigger on CS falling edge
    inst.write(":STOP")
    inst.write(":TRIGger:MODE EDGE")
    inst.write(":TRIGger:EDGE:SOURce CHANnel1")
    inst.write(":TRIGger:EDGE:LEVel 2.5")
    inst.write(":TRIGger:EDGE:SLOPE NEG")
    inst.write(":TRIGger:HOLDoff 0")
    time.sleep(0.3)

    print("Arming single trigger on CS falling edge...")
    inst.write(":SINGLE")
    print("Scope armed! Resetting Nano now...")

    ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
    ser.setDTR(False)
    time.sleep(0.2)
    ser.setDTR(True)
    ser.close()
    time.sleep(0.5)

    waves = {}
    times = None
    for ch in range(1, 5):
        print(f"Reading CH{ch}...", end=" ")
        samples, pre = read_waveform_raw(inst, ch)
        if len(samples) == 0:
            print("NO DATA")
            continue
        waves[ch] = samples
        voltages = [v for _, v in samples]
        print(f"{len(samples)} samples, V range: {min(voltages):.2f}V to {max(voltages):.2f}V")
        if times is None:
            times = [t for t, _ in samples]

    logic = {ch: to_logic(waves[ch], THRESHOLD_V) for ch in range(1, 5)}
    txns = decode_spi(logic[CH_CS], logic[CH_CLK], logic[CH_MOSI], logic[CH_MISO], times)

    print(f"\n=== Decoded SPI Transactions ({len(txns)}) ===")
    for txn in txns:
        print(f"  t={txn['t_start_us']}us: MOSI={' '.join(txn['mosi'])} MISO={' '.join(txn['miso'])}")

    inst.close()

if __name__ == "__main__":
    main()
