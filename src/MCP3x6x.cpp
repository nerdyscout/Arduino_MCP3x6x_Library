#include "MCP3x6x.h"

MCP3x6x::MCP3x6x(const uint16_t MCP3x6x_DEVICE_TYPE) {
  _id = MCP3x6x_DEVICE_TYPE;
#if (MCP3x6x_DEVICE_TYPE == MCP3461_DEVICE_TYPE)
  _resolution = 16;
  _channels   = 2;
#elif (MCP3x6x_DEVICE_TYPE == MCP3462_DEVICE_TYPE)
  _resolution = 16;
  _channels   = 4;
#elif (MCP3x6x_DEVICE_TYPE == MCP3464_DEVICE_TYPE)
  _resolution = 16;
  _channels   = 8;
#elif (MCP3x6x_DEVICE_TYPE == MCP3561_DEVICE_TYPE)
  _resolution = 24;
  _channels   = 2;
#elif (MCP3x6x_DEVICE_TYPE == MCP3562_DEVICE_TYPE)
  _resolution = 24;
  _channels   = 4;
#elif (MCP3x6x_DEVICE_TYPE == MCP3564_DEVICE_TYPE)
  _resolution = 24;
  _channels   = 8;
#endif
};

MCP3x6x::~MCP3x6x() { end(); };

bool MCP3x6x::begin(const uint8_t pinCS, SPIClass *theSPI) {
  _pinCS = pinCS;

  _spi   = theSPI;
  _spi->begin();
  return true;
}

bool MCP3x6x::begin(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS,
                    SPIClass *theSPI) {
  _pinIRQ  = pinIRQ;
  _pinMCLK = pinMCLK;
  _pinCS   = pinCS;

  _spi     = theSPI;
  _spi->begin();

  //  reset();
  //    settings.read();

  // ID is 0x0000 in MCP3x6x_DEFAULTS so if it differs now it has been read out from MCP3x6x
  //    if (settings.id._raw == MCP3x6x_DEVICE_TYPE) {
  //      return true;
  //    }

  pinMode(_pinIRQ, INPUT);
  pinMode(_pinMCLK, OUTPUT);  // TODO: this pin changes depending on CLK_SEK
  return true;
}

//bool MCP3x6x::available() { return _spi->initialized; }

void MCP3x6x::end() { _spi->end(); }

status_t MCP3x6x::_read(uint8_t *data, uint8_t addr, size_t size) {
  _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
  digitalWrite(_pinCS, LOW);
  _status.raw = _spi->transfer(addr);
  for (size_t i = 0; i < size; i++) {
    data[i] = _spi->transfer(0x00);
  }
  digitalWrite(_pinCS, HIGH);
  _spi->endTransaction();

  return _status;
}

status_t MCP3x6x::_write(uint8_t *data, uint8_t addr, size_t size) {
  _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
  digitalWrite(_pinCS, LOW);
  _status.raw = _spi->transfer(addr);
  for (size_t i = 0; i < size; i++) {
    _spi->transfer(data[i]);
  }
  digitalWrite(_pinCS, HIGH);
  _spi->endTransaction();

  return _status;
}

int32_t MCP3x6x::analogRead(uint8_t ch = 0x01) {
  uint8_t buffer[4];

  //    _selectMultiplexer(ch);
  //  conversion();
  /*
    switch (settings.config3.data_format) {
      case (MCP3x6x::data_format_e::sgn_data):
        _status = _read(buffer, (MCP3x6x_CMD_IREAD | MCP3x6x_ADR_ADCDATA), 3);
        break;
      default:
        _status = _read(buffer, (MCP3x6x_CMD_IREAD | MCP3x6x_ADR_ADCDATA), 4);
    }
*/
  // todo ensure signedness is correct
  return (int32_t)buffer & 0x00FFFFFF;
}