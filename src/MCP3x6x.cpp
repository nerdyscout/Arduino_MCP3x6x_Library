// SPDX-License-Identifier: MIT

/**
 * @file MCP3x6x.cpp
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.3
 * @date 2024-04-10
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "MCP3x6x.hpp"

#include <Arduino.h>

#include <cstring>
#ifdef ARDUINO_ARCH_SAMD
#  include <wiring_private.h>
#endif

MCP3x6x::MCP3x6x(const uint16_t MCP3x6x_DEVICE_TYPE, const uint8_t pinCS, SPIClass *theSPI,
                 SPISettings theSPISettings, const uint8_t pinMOSI, const uint8_t pinMISO,
                 const uint8_t pinCLK) {
  _spi         = theSPI;
  _spiSettings = theSPISettings;
  _pinMISO     = pinMISO;
  _pinMOSI     = pinMOSI;
  _pinCLK      = pinCLK;
  _pinCS       = pinCS;

  switch (MCP3x6x_DEVICE_TYPE) {
    case MCP3461_DEVICE_TYPE:
    case MCP3462_DEVICE_TYPE:
    case MCP3464_DEVICE_TYPE:
      _resolution = 16;
      break;
    case MCP3561_DEVICE_TYPE:
    case MCP3562_DEVICE_TYPE:
    case MCP3564_DEVICE_TYPE:
      _resolution = 24;
      break;
  }

  switch (MCP3x6x_DEVICE_TYPE) {
    case MCP3461_DEVICE_TYPE:
    case MCP3561_DEVICE_TYPE:
      _channel_count = 1;
      break;
    case MCP3462_DEVICE_TYPE:
    case MCP3562_DEVICE_TYPE:
      _channel_count = 2;
      break;
    case MCP3564_DEVICE_TYPE:
    case MCP3464_DEVICE_TYPE:
      _channel_count = 4;
      break;
  }

  _channel_mask |= 0xff << _channel_count;  // todo use this one
}

void MCP3x6x::_reverse_array(uint8_t *array, size_t size) {
  for (size_t i = 0, e = size; i <= e / 2; i++, e--) {
    uint8_t temp = array[i];
    array[i]     = array[e - 1];
    array[e - 1] = temp;
  }
}

/*
MCP3x6x::status_t MCP3x6x::_transfer16(uint8_t *data, uint8_t addr, size_t size = 2) {
  _spi->beginTransaction(_spiSettings);
  digitalWrite(_pinCS, LOW);

  if (bitRead(addr, 0)) {  // read
    _spi->transfer16(addr << 8);
    _spi->transfer16(*data);
  } else {  // write
    _spi->transfer16(addr << 8 | data[0]);
    _spi->transfer16(data[1] << 8 | data[2]);
  }

  digitalWrite(_pinCS, HIGH);
  _spi->endTransaction();

  return _status;
}
*/

MCP3x6x::status_t MCP3x6x::_transfer(uint8_t *data, uint8_t addr, size_t size) {
  _spi->beginTransaction(_spiSettings);
  digitalWrite(_pinCS, LOW);
  _status.raw = _spi->transfer(addr);
  if (size) _spi->transfer(data, size);
  digitalWrite(_pinCS, HIGH);
  _spi->endTransaction();

  return _status;
}

bool MCP3x6x::begin() {
  // setup SPI
#if defined(ARDUINO_ARCH_STM32)
  _spi = new SPIClass(_pinMOSI, _pinMISO, _pinCLK, _pinCS);
  _spi->begin();

#elif defined(ARDUINO_ARCH_ESP32)
  _spi->begin(_pinCLK, _pinMISO, _pinMOSI, _pinCS);

#else
  pinMode(_pinCS, OUTPUT);
  digitalWrite(_pinCS, HIGH);

  _spi->begin();
#  if ARDUINO_ARCH_SAMD
  pinPeripheral(_pinMISO, PIO_SERCOM);
  pinPeripheral(_pinMOSI, PIO_SERCOM);
  pinPeripheral(_pinCLK, PIO_SERCOM);
#  endif

#endif

  // reset ADC
  bool s = reset().por;
  configure();

  return s;
}

void MCP3x6x::configure(const Config0 config0, const Config1 config1, const Config2 config2,
                        const Config3 config3, const Irq irq, const Mux mux, const Scan scan,
                        const Timer timer, const Offset offset, const Gain gain, const Lock lock,
                        const Crccfg crccfg) {
  if (_config0.raw != config0.raw) write(_config0 = config0);
  if (_config1.raw != config1.raw) write(_config1 = config1);
  if (_config2.raw != config2.raw) write(_config2 = config2);
  if (_config3.raw != config3.raw) write(_config3 = config3);
  if (_irq.raw != MCP3x6x_CFG_IRQ) write(irq);
  if (_mux.raw != MCP3x6x_CFG_MUX) write(mux);
  if (memcmp(_scan.raw, MCP3x6x_CFG_SCAN, sizeof(scan))) write(_scan = scan);
  if (memcmp(_timer.raw, MCP3x6x_CFG_TIMER, sizeof(timer))) write(_timer = timer);
  if (memcmp(_offset.raw, MCP3x6x_CFG_OFFSET, sizeof(offset))) write(_offset = offset);
  if (memcmp(_gain.raw, MCP3x6x_CFG_GAIN, sizeof(gain))) write(_gain = gain);
  if (_lock.raw != MCP3x6x_CFG_LOCK) write(_lock = lock);
  if (memcmp(_crccfg.raw, MCP3x6x_CFG_CRCCFG, sizeof(crccfg))) write(_crccfg = crccfg);
}

MCP3x6x::status_t MCP3x6x::read(Adcdata *data) {
  size_t s = 1;

  switch (_resolution) {
    case 16:
      s = _config3.data_format == data_format::SGN_DATA ? 2 : 4;
      break;
    case 24:
      s = _config3.data_format == data_format::SGN_DATA ? 3 : 4;
      break;
  }

  uint8_t buffer[s];
  memset(buffer, 0, s);

  //  while (status_dr()) {
  _transfer(buffer, MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, s);
  //  }

  _reverse_array(buffer, s);

  data->channelid = _getChannel((uint32_t &)buffer);
  data->value     = _getValue((uint32_t &)buffer);

  return _status;
}

void MCP3x6x::config0(enum adc_mode adc, enum cs_sel bias, enum clk_sel clk, bool vref_sel) {
  _config0.adc      = adc;
  _config0.bias     = bias;
  _config0.clk      = clk;
  _config0.vref_sel = vref_sel;

  write(_config0);
}

void MCP3x6x::config1(enum osr osr, enum pre pre) {
  _config1.osr = osr;
  _config1.pre = pre;
  write(_config1);
}

void MCP3x6x::config2(bool az_mux, enum gain gain, enum boost boost) {
  _config2.az_mux = az_mux;
  _config2.gain   = gain;
  _config2.boost  = boost;
  write(_config2);
}

void MCP3x6x::config3(bool gaincal, bool offcal, bool crccom, enum data_format data_format,
                      enum conv_mode conv_mode) {
  _config3.en_gaincal  = gaincal;
  _config3.en_offcal   = offcal;
  _config3.en_crccom   = crccom;
  _config3.data_format = data_format;
  _config3.conv_mode   = conv_mode;
  write(_config3);
}

void MCP3x6x::irq(bool stp, bool fastcmd, uint8_t irq_mode) {
  _irq.en_stp     = stp;
  _irq.en_fastcmd = fastcmd;
  _irq.irq_mode   = irq_mode;
  write(_irq);
}

void MCP3x6x::mux(enum mux minus, enum mux plus) {
  _mux.vin_minus = minus;
  _mux.vin_plus  = plus;
  write(_mux);
}

void MCP3x6x::scan(byte single_ended, byte differential, bool temp, bool avdd, bool vcm,
                   bool offset, enum delay dly) {
  _scan.channel.single_ended = single_ended;
  _scan.channel.differential = differential;
  _scan.channel.temp         = temp;
  _scan.channel.avdd         = avdd;
  _scan.channel.vcm          = vcm;
  _scan.channel.offset       = offset;
  _scan.dly                  = dly;
  write(_scan);
}

void MCP3x6x::timer(uint8_t *timer) {
  _timer = timer;
  write(_timer);
}

void MCP3x6x::offset(uint8_t *offset) {
  _offset = offset;
  write(_offset);
}

void MCP3x6x::gain(uint8_t *gain) {
  _gain = gain;
  write(_gain);
}

void MCP3x6x::lock(uint8_t key = MCP3x6x_CFG_LOCK) {
  _lock.raw = key;
  write(_lock);
}

void MCP3x6x::unlock() {
  // todo
  write(_lock);
}

/**
 * @brief write register CRCCFG
 *
 * @param crccfg
 */
void MCP3x6x::crccfg(uint16_t crccfg) {
  _crccfg.value = crccfg;
  write(_crccfg);
}

void MCP3x6x::IRQ_handler() {
  //  _read(&_adcdata, MCP3x6x_ADR_ADCDATA, 4);
  _result.raw[(uint8_t)_adcdata.channelid] = _adcdata.value;
}

void MCP3x6x::setDataFormat(data_format format) {
  _config3.data_format = format;
  write(_config3);

  switch (format) {
    case data_format::SGN_DATA:
    case data_format::SGN_DATA_ZERO:
      _resolution--;
      break;
    case data_format::SGNEXT_DATA:
    case data_format::ID_SGNEXT_DATA:
      break;
    default:
      //      _resolution = -1;
      break;
  }
}

void MCP3x6x::setConversionMode(conv_mode mode) {
  _config3.conv_mode = mode;
  write(_config3);
}

void MCP3x6x::setAdcMode(adc_mode mode) {
  _config0.adc = mode;
  write(_config0);
}

void MCP3x6x::setClockSelection(clk_sel clk) {
  _config0.clk = clk;
  write(_config0);
}

void MCP3x6x::enableScanChannel(Mux ch) {
  for (size_t i = 0; i < sizeof(_channelID); i++) {
    if (_channelID[i] == ch.raw) {
      bitSet(_scan.channel.raw, i);
      break;
    }
  }
  write(_scan);
}

void MCP3x6x::disableScanChannel(Mux ch) {
  for (size_t i = 0; i < sizeof(_channelID); i++) {
    if (_channelID[i] == ch.raw) {
      bitClear(_scan.channel.raw, i);
      break;
    }
  }
  write(_scan);
}

void MCP3x6x::setReference(float vref) {
  if (vref == 0.0) {
    vref              = 2.4;
    _config0.vref_sel = 1;
    write(_config0);
  }
  _reference = vref;
}

float MCP3x6x::getReference() { return _reference; }

// returns signed ADC value from raw data
int32_t MCP3x6x::_getValue(int32_t raw) {
  switch (_resolution) {
    case 16:
      switch (_config3.data_format) {
        case (data_format::SGN_DATA_ZERO):
          return raw >> 16;
        case (data_format::SGN_DATA):
          bitWrite(raw, 31, bitRead(raw, 16));
          bitClear(raw, 16);
          return raw;
        case (data_format::SGNEXT_DATA):
        case (data_format::ID_SGNEXT_DATA):
          bitWrite(raw, 31, bitRead(raw, 17));
          return raw & 0x8000FFFF;
      }
      break;

    case 24:
      switch (_config3.data_format) {
        case (data_format::SGN_DATA_ZERO):
          return raw >> 8;
        case (data_format::SGN_DATA):
          bitWrite(raw, 31, bitRead(raw, 24));
          bitClear(raw, 24);
          return raw;
        case (data_format::SGNEXT_DATA):
        case (data_format::ID_SGNEXT_DATA):
          bitWrite(raw, 31, bitRead(raw, 25));
          return raw & 0x80FFFFFF;
      }
      break;
  }

  return -1;
}

uint8_t MCP3x6x::_getChannel(uint32_t raw) {
  if (_config3.data_format == data_format::ID_SGNEXT_DATA) {
    return ((raw >> 28) & 0x0F);
  } else {
    for (size_t i = 0; i < sizeof(_channelID); i++) {
      if (_channelID[i] == _mux.raw) {
        return i;
      }
    }
  }
  return -1;
}

int32_t MCP3x6x::analogRead(Mux ch) {
  // MuxMode
  if (_scan.channel.raw == 0) {
    _mux = ch;
    write(_mux);
    conversion();
    //    _read(&_adcdata, MCP3x6x_ADR_ADCDATA, 4);

    return _result.raw[(uint8_t)_adcdata.channelid] = _adcdata.value;
  }

  // ScanMode
  for (size_t i = 0; i < sizeof(_channelID); i++) {
    if (_channelID[i] == ch.raw) {
      conversion();
      while (status_dr()) {
        //        _read(&_adcdata.raw, MCP3x6x_ADR_ADCDATA, 4);
      }

      return _adcdata.value;
    }
  }
  return -1;
}

int32_t MCP3x6x::analogReadDifferential(Mux pinP, Mux pinN) {
  _mux = ((uint8_t)pinP.raw << 4) | (uint8_t)pinN.raw;
  write(_mux);

  conversion();
  //  _read(&_adcdata, MCP3x6x_ADR_ADCDATA, 4);
  return _result.raw[(uint8_t)_adcdata.channelid] = _adcdata.value;
}

void MCP3x6x::analogReadResolution(size_t bits) {
  if (bits <= _resolution) {
    _resolution = bits;
  }
}

void MCP3x6x::setResolution(size_t bits) { analogReadResolution(bits); }

uint32_t MCP3x6x::getMaxValue() { return pow(2, _resolution); }

bool MCP3x6x::isComplete() { return status_dr(); }

void MCP3x6x::startContinuous() {
  setConversionMode(conv_mode::CONTINUOUS);
  conversion();
}

void MCP3x6x::stopContinuous() {
  setConversionMode(conv_mode::ONESHOT_STANDBY);
  standby();
}

void MCP3x6x::startContinuousDifferential() {
  differentialMode();
  startContinuous();
}

bool MCP3x6x::isContinuous() {
  if (_config3.conv_mode == conv_mode::CONTINUOUS) {
    return true;
  }
  return false;
}

void MCP3x6x::setAveraging(osr rate) {
  _config1.osr = rate;
  write(_config1);
}

int32_t MCP3x6x::analogReadContinuous(Mux ch) {
  if (isContinuous()) {
    for (size_t i = 0; i < sizeof(_channelID); i++) {
      if (_channelID[i] == ch.raw) {
        return _result.raw[(uint8_t)_adcdata.channelid];
      }
    }
  }
  return -1;
}

void MCP3x6x::attachIRQ(const uint8_t pinIRQ, void (*callback)(void)) {
  _pinIRQ = pinIRQ;

  attachInterrupt(digitalPinToInterrupt(_pinIRQ), callback, FALLING);
}

void MCP3x6x::attachMCLK(const uint8_t pinMCLK) {
  _pinMCLK = pinMCLK;

#if ((F_CPU / 2) < 4915200)
#  error "MCLK frequency is too low"
#else
  tone(_pinMCLK, 4915200);
#endif
}
