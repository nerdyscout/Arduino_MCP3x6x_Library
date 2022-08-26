#include "MCP3x6x.h"

#include <wiring_private.h>

MCP3x6x::MCP3x6x(const uint16_t MCP3x6x_DEVICE_TYPE) {
  //  settings.id.raw = MCP3x6x_DEVICE_TYPE;

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

/*
MCP3x6x::status_t MCP3x6x::_readADC(uint32_t *buffer) {
  _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
  digitalWrite(_pinCS, LOW);
  _status.raw = _spi->transfer(MCP3x6x_ADR_ADCDATA | MCP3x6x_CMD_SREAD);
  for (size_t i = 0; i < 4; i++) {
    //    (uint8_t )&buffer = _spi->transfer(0x00);
  }
  digitalWrite(_pinCS, HIGH);
  _spi->endTransaction();
 
 return _status;
}
*/
/*
void MCP3x6x::_irq_handler() {
  /*

  //todo
  byte data[4];
  _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
  digitalWrite(_pinCS, LOW);
  _status.raw = _spi->transfer(0x0);
  for (size_t i = 0; i < 4; i++) {
    data[i] = _spi->transfer(0x00);
  }

  digitalWrite(_pinCS, HIGH);
  _spi->endTransaction();
*/
//}

bool MCP3x6x::begin(const uint8_t SPI_CS, SPIClass *theSPI, uint8_t SPI_MOSI, uint8_t SPI_MISO, uint8_t SPI_CLK) {
  _pinCS = SPI_CS;
  pinMode(_pinCS, OUTPUT);
  digitalWrite(_pinCS, HIGH);

  _spi = theSPI;
  _spi->begin();
  pinPeripheral(SPI_MISO, PIO_SERCOM);
  pinPeripheral(SPI_MOSI, PIO_SERCOM);
  pinPeripheral(SPI_CLK, PIO_SERCOM);

  _status              = reset();
  _status              = read(settings);

  settings.config0.clk = clk_sel::internal;
  _status              = write(settings.config0);

  return true;
}

bool MCP3x6x::begin(const uint8_t pinIRQ_MDAT, const uint8_t pinMCLK, bool MCLK_direction, uint8_t SPI_CS, SPIClass *theSPI,
                    uint8_t SPI_MOSI, uint8_t SPI_MISO, uint8_t SPI_CLK) {
  _pinCS = SPI_CS;
  pinMode(_pinCS, OUTPUT);
  digitalWrite(_pinCS, HIGH);

  _spi = theSPI;
  _spi->begin();
  pinPeripheral(SPI_MISO, PIO_SERCOM);
  pinPeripheral(SPI_MOSI, PIO_SERCOM);
  pinPeripheral(SPI_CLK, PIO_SERCOM);

  _status = reset();
  _status = read(settings);

  return true;
}

MCP3x6x::status_t MCP3x6x::_transfer(uint8_t *data, uint8_t addr, size_t size) {
  _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
  digitalWrite(_pinCS, LOW);
  _status.raw = _spi->transfer(addr);
  _spi->transfer(data, size);
  digitalWrite(_pinCS, HIGH);
  _spi->endTransaction();

  return _status;
}

MCP3x6x::status_t MCP3x6x::_fastcmd(uint8_t cmd) {
  _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
  digitalWrite(_pinCS, LOW);
  _status.raw = _spi->transfer(cmd);
  digitalWrite(_pinCS, HIGH);
  _spi->endTransaction();
  return _status;
}

int32_t MCP3x6x::analogRead(uint8_t channel) {
  //  uint32_t adcvalue;

  if (settings.scan.channels.raw == 0x0000) {
    // muxmode

    /*    while (!_status.dr) {
      // poll for new data
//      _status = read(&adcvalue, MCP3x6x_ADR_ADCDATA, 4);
    }
    */

  } else {
    // scanmode
  }

  /*
  switch (settings.config3.data_format) {
    case (data_format::sgn_data):  // default
      _status = _read((uint8_t *)_adcdata.ch[single_channel], (MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA), 3);
      break;
    case (data_format::sgn_data_zero):
      _status = _read((uint8_t *)_adcdata.ch[single_channel], (MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA), 4);
      //        _adcdata.ch[ch] >>= 8;
      break;
    case (data_format::sgnext_data):
    case (data_format::id_sgnext_data):
      _status = _read((uint8_t *)_adcdata.ch[single_channel], (MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA), 4);
      //        _adcdata.ch[ch] = bitRead(_adcdata.ch[ch], 24) ?
      bitSet(_adcdata.ch[single_channel], 31);
      break;
  };

*/

  //  return _adcdata.ch[single_channel] & 0x80FFFFFF;

  return 0x5a;
}

/*

MCP3x6x::Mux MCP3x6x::channel2mux(channelID ch) {
  Mux mask;
  mask.raw = 0;

  return mask;
}


MCP3x6x::Scan MCP3x6x::channel2scan(channelID ch) {
  Scan mask;
  mask.raw = 0;

 return mask;
}

*/

void MCP3x6x::setMode(uint8_t mode) {
  switch (mode) {
    case MCP3x6x_MODE_CONTINUOUS_CONVERSION:
      settings.config0.adc       = MCP3x6x::adc_mode::conversion;
      settings.config3.conv_mode = MCP3x6x::conv_mode::continuous;
      break;
    case MCP3x6x_MODE_ONESHOT_STANDBY:
      settings.config0.adc       = MCP3x6x::adc_mode::standby;
      settings.config3.conv_mode = MCP3x6x::conv_mode::oneshot_standby;
    case MCP3x6x_MODE_ONESHOT_SHUTDOWN:
      settings.config0.adc       = MCP3x6x::adc_mode::shutdown;
      settings.config3.conv_mode = MCP3x6x::conv_mode::oneshot_shutdown;
  }

  settings.config0.clk         = MCP3x6x::clk_sel::internal;
  settings.config3.data_format = MCP3x6x::data_format::id_sgnext_data;
  settings.config3.en_gaincal  = true;
  settings.config3.en_offcal   = true;
  write(settings.config0);
  write(settings.config3);

  //  write(settings); // dont know why this one is not working
}