// SPDX-License-Identifier: MIT

/**
 * @file MCP3x6x.cpp
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.2
 * @date 2023-10-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "MCP3x6x.hpp"

#include <Arduino.h>

#ifdef ARDUINO_ARCH_SAMD
#  include <wiring_private.h>
#endif

MCP3x6x::MCP3x6x(const uint16_t MCP3x6x_DEVICE_TYPE, const uint8_t pinCS, SPIClass *theSPI,
                 const uint8_t pinMOSI, const uint8_t pinMISO, const uint8_t pinCLK)
    : _settings(MCP3x6x_DEVICE_TYPE) {
  _spi        = theSPI;
  _pinMISO    = pinMISO;
  _pinMOSI    = pinMOSI;
  _pinCLK     = pinCLK;
  _pinCS      = pinCS;

  _resolution = getMaxResolution();
  _channel_mask |= 0xff << getChannelCount();  // todo use this one
}

MCP3x6x::MCP3x6x(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint16_t MCP3x6x_DEVICE_TYPE,
                 const uint8_t pinCS, SPIClass *theSPI, const uint8_t pinMOSI,
                 const uint8_t pinMISO, const uint8_t pinCLK)
    : MCP3x6x(MCP3x6x_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK) {
  _pinIRQ  = pinIRQ;
  _pinMCLK = pinMCLK;

  attachInterrupt(digitalPinToInterrupt(_pinIRQ), mcp_wrapper, FALLING);
}

void MCP3x6x::_reverse_array(uint8_t *array, size_t size) {
  for (size_t i = 0, e = size; i <= e / 2; i++, e--) {
    uint8_t temp = array[i];
    array[i]     = array[e - 1];
    array[e - 1] = temp;
  }
}

MCP3x6x::status_t MCP3x6x::_transfer(uint8_t *data, uint8_t addr, size_t size) {
  _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
  noInterrupts();
  digitalWrite(_pinCS, LOW);
  _status.raw = _spi->transfer(addr);
  _spi->transfer(data, size);
  digitalWrite(_pinCS, HIGH);
  interrupts();
  _spi->endTransaction();

  return _status;
}

bool MCP3x6x::begin(MCP3x6x::Settings settings) {
  //  _settings = settings;

  pinMode(_pinCS, OUTPUT);
  digitalWrite(_pinCS, HIGH);

  _spi->begin();

#if ARDUINO_ARCH_SAMD
  // todo figure out how to get dynamicaly sercom index
  //  uint index = _spi->getSercomIndex();
  pinPeripheral(_pinMISO, PIO_SERCOM);
  pinPeripheral(_pinMOSI, PIO_SERCOM);
  pinPeripheral(_pinCLK, PIO_SERCOM);
#endif

  reset();
}

bool MCP3x6x::begin(uint16_t channelmask, float vref) {
  pinMode(_pinCS, OUTPUT);
  digitalWrite(_pinCS, HIGH);

  _spi->begin();

#if ARDUINO_ARCH_SAMD
  // todo figure out how to get dynamicaly sercom index
  //  uint index = _spi->getSercomIndex();
  pinPeripheral(_pinMISO, PIO_SERCOM);
  pinPeripheral(_pinMOSI, PIO_SERCOM);
  pinPeripheral(_pinCLK, PIO_SERCOM);
#endif

  reset();
  setClockSelection(clk_sel::INTERN);          // todo make configurable by function parameter
  setDataFormat(data_format::ID_SGNEXT_DATA);  // todo make configurable by function parameter

  // scanmode
  if (channelmask != 0) {
    _settings.scan.channel.raw = channelmask;  // todo apply _channel_mask
    write(_settings.scan);
  }

  setReference(vref);
  standby();

  return true;
}

MCP3x6x::status_t MCP3x6x::read(Adcdata *data) {
  size_t s = 0;

  switch (getMaxResolution()) {
    case 16:
      s = _settings.config3.data_format == data_format::SGN_DATA ? 2 : 4;
      break;
    case 24:
      s = _settings.config3.data_format == data_format::SGN_DATA ? 3 : 4;
      break;
  }

  uint8_t buffer[s];
  _transfer(buffer, MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, s);
  _reverse_array(buffer, s);

  data->channelid = _getChannel((uint32_t &)buffer);
  data->value     = _getValue((uint32_t &)buffer);

  return _status;
}

void MCP3x6x::IRQ_handler() {
  read(&adcdata);
  result.raw[(uint8_t)adcdata.channelid] = adcdata.value;
}

void MCP3x6x::lock(uint8_t key) {
  _settings.lock.raw = key;
  write(_settings.lock);
}

void MCP3x6x::unlock() {
  _settings.lock.raw = _settings._defaults.LOCK;
  write(_settings.lock);
}

void MCP3x6x::setDataFormat(data_format format) {
  _settings.config3.data_format = format;
  write(_settings.config3);

  switch (format) {
    case data_format::SGN_DATA:
    case data_format::SGN_DATA_ZERO:
      _resolution--;
      break;
    case data_format::SGNEXT_DATA:
    case data_format::ID_SGNEXT_DATA:
      break;
    default:
      _resolution = -1;
      break;
  }
}

void MCP3x6x::setConversionMode(conv_mode mode) {
  _settings.config3.conv_mode = mode;
  write(_settings.config3);
}

void MCP3x6x::setAdcMode(adc_mode mode) {
  _settings.config0.adc = mode;
  write(_settings.config0);
}

void MCP3x6x::setClockSelection(clk_sel clk) {
  _settings.config0.clk = clk;
  write(_settings.config0);
}

void MCP3x6x::enableScanChannel(mux_t ch) {
  for (size_t i = 0; i < sizeof(_channelID); i++) {
    if (_channelID[i] == ch.raw) {
      bitSet(_settings.scan.channel.raw, i);
      break;
    }
  }
  write(_settings.scan);
}

void MCP3x6x::disableScanChannel(mux_t ch) {
  for (size_t i = 0; i < sizeof(_channelID); i++) {
    if (_channelID[i] == ch.raw) {
      bitClear(_settings.scan.channel.raw, i);
      break;
    }
  }
  write(_settings.scan);
}

void MCP3x6x::setReference(float vref) {
  if (vref == 0.0) {
    vref                       = 2.4;
    _settings.config0.vref_sel = 1;
    write(_settings.config0);
  }
  _reference = vref;
}

float MCP3x6x::getReference() { return _reference; }

// returns signed ADC value from raw data
int32_t MCP3x6x::_getValue(uint32_t raw) {
  switch (getMaxResolution()) {
    case 16:
      switch (_settings.config3.data_format) {
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
      switch (_settings.config3.data_format) {
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
  if (_settings.config3.data_format == data_format::ID_SGNEXT_DATA) {
    return ((raw >> 28) & 0x0F);
  } else {
    for (size_t i = 0; i < sizeof(_channelID); i++) {
      if (_channelID[i] == _settings.mux.raw) {
        return i;
      }
    }
  }
  return -1;
}

int32_t MCP3x6x::analogRead(mux_t ch) {
  // MuxMode
  if (_settings.scan.channel.raw == 0) {
    _settings.mux = ch;
    write(_settings.mux);
    conversion();
    while (status_dr()) {
      read(&adcdata);
    }

    return result.raw[(uint8_t)adcdata.channelid] = adcdata.value;
  }

  // ScanMode
  for (size_t i = 0; i < sizeof(_channelID); i++) {
    if (_channelID[i] == ch.raw) {
      conversion();
      while (status_dr()) {
        read(&adcdata);
      }

      return adcdata.value;
    }
  }
  return -1;
}

int32_t MCP3x6x::analogReadDifferential(mux pinP, mux pinN) {
  _settings.mux = ((uint8_t)pinP << 4) | (uint8_t)pinN;
  write(_settings.mux);

  conversion();
  read(&adcdata);
  return result.raw[(uint8_t)adcdata.channelid] = adcdata.value;
}

void MCP3x6x::analogReadResolution(size_t bits) {
  if (bits <= getMaxResolution()) {
    _resolution = bits;
  }
}

void MCP3x6x::setResolution(size_t bits) { analogReadResolution(bits); }

void MCP3x6x::singleEndedMode() { _differential = false; }

void MCP3x6x::differentialMode() { _differential = true; }

bool MCP3x6x::isDifferential() { return _differential; }

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
  if (_settings.config3.conv_mode == conv_mode::CONTINUOUS) {
    return true;
  }
  return false;
}

void MCP3x6x::setAveraging(osr rate) {
  _settings.config1.osr = rate;
  write(_settings.config1);
}

int32_t MCP3x6x::analogReadContinuous(mux_t ch) {
  if (isContinuous()) {
    for (size_t i = 0; i < sizeof(_channelID); i++) {
      if (_channelID[i] == ch.raw) {
        return result.raw[(uint8_t)adcdata.channelid];
      }
    }
  }
  return -1;
}
