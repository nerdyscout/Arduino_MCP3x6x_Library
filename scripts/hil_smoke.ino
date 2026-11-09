// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// hil_smoke.ino — Hardware-in-the-loop smoke sketch for Arduino Nano (ATmega328P).
// Drives MCP3x6x over SPI and prints results to Serial at 115200 baud for
// correlation with oscilloscope SPI captures (CS=10, MOSI=11, MISO=12, SCK=13).
//
// Usage:
//   pio run -e ATMEGA328P -d <scratch_dir> && pio upload -e ATMEGA328P -d <scratch_dir>
//
// Scope channel mapping (for decoder.py readback):
//   CS   = pin 10  -> CHAN1
//   SCLK = pin 13  -> CHAN3
//   MISO = pin 12  -> CHAN2
//   MOSI = pin 11  -> CHAN4
//

#include <Arduino.h>
#include <SPI.h>
#include "Arduino_MCP3x6x_Library.hpp"

MCP3564 mcp;

static void printStatus(const char* where) {
  Serial.print(where);
  Serial.print(": DR=");
  Serial.print(mcp.status_dr());
  Serial.print(" CRCCFG=");
  Serial.print(mcp.status_crccfg());
  Serial.print(" POR=");
  Serial.print(mcp.status_por());
  Serial.print(" ADDR=");
  Serial.println(mcp.getDeviceAddress());
}

void setup() {
#if defined(USB_VID) && defined(USB_PID)
  while (!Serial);
#else
  delay(2000);
#endif

  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  Serial.print("begin() ... ");
  bool ok = mcp.begin();
  Serial.println(ok ? "OK" : "FAILED");

  if (!ok) {
    Serial.println("Chip ID mismatch — check part marking.");
    Serial.println("Expected chip-ID nibble: 0xF (MCP3564, 24-bit 8ch)");
    while (1);
  }

  Serial.print("Device address: 0x");
  Serial.println(mcp.getDeviceAddress(), HEX);
  Serial.println("Hardware config test sequence starting...");
  Serial.println();

  // Unlock
  mcp.unlock();
  Serial.println("PASS: unlock() succeeded");

  // Config register writes
  mcp.config0(MCP3564::CONVERSION, MCP3564::BIAS_0UA, MCP3564::EXTERN, false);
  Serial.println("PASS: config0(CONVERSION, BIAS_0UA, EXTERN) succeeded");

  mcp.config1(MCP3564::OSR_256, MCP3564::MCLK_0);
  Serial.println("PASS: config1(OSR_256, MCLK_0) succeeded");

  mcp.config2(false, MCP3564::GAIN_1, MCP3564::BOOST_2);
  Serial.println("PASS: config2(GAIN_1, BOOST_2) succeeded");

  mcp.config3(false, false, false, MCP3564::SGN_DATA, MCP3564::CONTINUOUS);
  Serial.println("PASS: config3(SGN_DATA, CONTINUOUS) succeeded");

  mcp.irq(false, false, MCP3564::IRQ_OFF);
  Serial.println("PASS: irq(IRQ_OFF) succeeded");

  mcp.mux(MCP3564::MUX_CH0, MCP3564::MUX_AGND);
  Serial.println("PASS: mux(CH0, AGND) succeeded");

  printStatus("after config");

  // Poll analogRead on all channels
  Serial.println("Polling analogRead(CH0..CH3):");
  int32_t raw0 = mcp.analogRead(MCP3x6x_CH0);
  int32_t uV0  = mcp.toVoltage(raw0);
  Serial.print("CH0: raw=");
  Serial.print(raw0);
  Serial.print(" uV=");
  Serial.println(uV0);

  int32_t raw1 = mcp.analogRead(MCP3x6x_CH1);
  int32_t uV1  = mcp.toVoltage(raw1);
  Serial.print("CH1: raw=");
  Serial.print(raw1);
  Serial.print(" uV=");
  Serial.println(uV1);

  int32_t raw2 = mcp.analogRead(MCP3x6x_CH2);
  int32_t uV2  = mcp.toVoltage(raw2);
  Serial.print("CH2: raw=");
  Serial.print(raw2);
  Serial.print(" uV=");
  Serial.println(uV2);

  int32_t raw3 = mcp.analogRead(MCP3x6x_CH3);
  int32_t uV3  = mcp.toVoltage(raw3);
  Serial.print("CH3: raw=");
  Serial.print(raw3);
  Serial.print(" uV=");
  Serial.println(uV3);

  Serial.println("Hardware config test complete — scope capture complete.");
  printStatus("final");
}

void loop() {
  printStatus("status");
  delay(1000);
}
