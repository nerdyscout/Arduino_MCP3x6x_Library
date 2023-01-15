// SPDX-License-Identifier: MIT

/**
 * @file MCP3x6x.h
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.2
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef MCP3x6x_H
#define MCP3x6x_H

#define MCP3x6x_DEBUG DEBUG  //!< sets debug active

#include "MCPChannel.h"
#include "MCPSettings.h"

#if ARDUINO >= 100
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif
#include <SPI.h>

#ifndef MCP3x6x_DEVICE_ADDRESS
#  define MCP3x6x_DEVICE_ADDRESS (0x01)  //!< DEVICE ADDRESS
#endif
#ifndef MCP3x6x_SPI_SPEED
#  define MCP3x6x_SPI_SPEED (2000000)  //!< SPI SPEED Value
#endif
#if MCP3x6x_SPI_SPEED > 20000000
#  error "MCP3x6x_SPI_SPEED above 20MHz!"
#endif
#define MCP3x6x_SPI_ADR   (MCP3x6x_DEVICE_ADDRESS << 6)  //!< SPI ADDRESS
#define MCP3x6x_SPI_ORDER MSBFIRST                       //!< SPI ORDER
#define MCP3x6x_SPI_MODE  SPI_MODE0                      //!< SPI MODE

#define MCP3x6x_CMD_CONVERSION    (MCP3x6x_SPI_ADR | 0b101000)  //!< fast command
#define MCP3x6x_CMD_STANDBY       (MCP3x6x_SPI_ADR | 0b101100)  //!< fast command
#define MCP3x6x_CMD_SHUTDOWN      (MCP3x6x_SPI_ADR | 0b110000)  //!< fast command
#define MCP3x6x_CMD_FULL_SHUTDOWN (MCP3x6x_SPI_ADR | 0b110100)  //!< fast command
#define MCP3x6x_CMD_RESET         (MCP3x6x_SPI_ADR | 0b111000)  //!< fast command
#define MCP3x6x_CMD_SREAD         (MCP3x6x_SPI_ADR | 0b01)      //!< fast command
#define MCP3x6x_CMD_IREAD         (MCP3x6x_SPI_ADR | 0b11)      //!< fast command
#define MCP3x6x_CMD_IWRITE        (MCP3x6x_SPI_ADR | 0b10)      //!< fast command

#define MCP3x6x_ADR_ADCDATA   (MCP3x6x_SPI_ADR | (0x0 << 2))  //!< Register ADCDdata address
#define MCP3x6x_ADR_CONFIG0   (MCP3x6x_SPI_ADR | (0x1 << 2))  //!< Register Config0 address
#define MCP3x6x_ADR_CONFIG1   (MCP3x6x_SPI_ADR | (0x2 << 2))  //!< Register Config1 address
#define MCP3x6x_ADR_CONFIG2   (MCP3x6x_SPI_ADR | (0x3 << 2))  //!< Register Config2 address
#define MCP3x6x_ADR_CONFIG3   (MCP3x6x_SPI_ADR | (0x4 << 2))  //!< Register Config3 address
#define MCP3x6x_ADR_IRQ       (MCP3x6x_SPI_ADR | (0x5 << 2))  //!< Register IRQ address
#define MCP3x6x_ADR_MUX       (MCP3x6x_SPI_ADR | (0x6 << 2))  //!< Register MUX address
#define MCP3x6x_ADR_SCAN      (MCP3x6x_SPI_ADR | (0x7 << 2))  //!< Register SCAN address
#define MCP3x6x_ADR_TIMER     (MCP3x6x_SPI_ADR | (0x8 << 2))  //!< Register Timer address
#define MCP3x6x_ADR_OFFSET    (MCP3x6x_SPI_ADR | (0x9 << 2))  //!< Register OFFSET address
#define MCP3x6x_ADR_GAIN      (MCP3x6x_SPI_ADR | (0xA << 2))  //!< Register GAIN address
#define MCP3x6x_ADR_RESERVED1 (MCP3x6x_SPI_ADR | (0xB << 2))  //!< Register
#define MCP3x6x_ADR_RESERVED2 (MCP3x6x_SPI_ADR | (0xC << 2))  //!< Register
#define MCP3x6x_ADR_LOCK      (MCP3x6x_SPI_ADR | (0xD << 2))  //!< Register LOCK address
#define MCP3x6x_ADR_RESERVED3 (MCP3x6x_SPI_ADR | (0xE << 2))  //!< Register
#define MCP3x6x_ADR_CRCCFG    (MCP3x6x_SPI_ADR | (0xF << 2))  //!< Register CRCCFG address

/**
 * @brief base class
 *
 */
class MCP3x6x {
  typedef union {
    struct {
      struct __attribute__((__packed__)) {
        bool por    : 1;  //!< status: power on reset
        bool crccfg : 1;  //!< status: crc
        bool dr     : 1;  //!< status: data ready
      };
      uint8_t      : 1;  //!< !addr[0]
      uint8_t addr : 2;  //!< addresse
      uint8_t      : 2;  //!< EMTPY
    };
    uint8_t raw;
  } status_t;
  status_t _status;

  status_t _fastcmd(uint8_t cmd) { return _transfer(0x00, cmd, 0); }
  void _reverse_array(uint8_t *array, size_t size);
  status_t _transfer(uint8_t *data, uint8_t addr, size_t size = 1);
  virtual int32_t _getValue(uint32_t raw);
  uint8_t _getChannel(uint32_t raw);

  SPIClass *_spi;
  uint8_t _pinMISO, _pinMOSI, _pinCLK, _pinCS;
  uint8_t _pinMCLK, _pinIRQ;

  bool _differential = false;
  float _reference   = 3.3;

  MCPChannel *_channel;

 public:
  MCPSettings settings;

  /**
   * @brief Construct a new MCP3x6x object
   *
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3x6x(const uint8_t pinCS, SPIClass *theSPI, const uint8_t pinMOSI, const uint8_t pinMISO,
          const uint8_t pinCLK, MCPChannel *channel);

  /**
   * @brief Construct a new MCP3x6x object
   *
   * @param pinIRQ
   * @param pinMCLK
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3x6x(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS, SPIClass *theSPI,
          const uint8_t pinMOSI, const uint8_t pinMISO, const uint8_t pinCLK, MCPChannel *channel);

  /**
   * @brief Destroy the MCP3x6x object
   *
   */
  //  ~MCP3x6x() { end(); };

  /**
   * @brief begin communication
   *
   * @param MCPSettings
   * @return true
   * @return false
   */
  bool begin(MCPSettings settings);

  /**
   * @brief end communication
   *
   */
  void end() { _spi->end(); }

  /**
   * @brief data ready status of latest communication
   *
   * @return true
   * @return false
   */
  inline bool status_dr() { return _status.dr; }

  /**
   * @brief crccfg status of latest communication
   *
   * @return true
   * @return false
   */
  inline bool status_crccfg() { return _status.crccfg; }

  /**
   * @brief power on reset status of latest communication
   *
   * @return true
   * @return false
   */
  inline bool status_por() { return _status.por; }

  /**
   * @brief Fast Command
   *
   * @return status_t
   */
  inline status_t conversion() { return _fastcmd(MCP3x6x_CMD_CONVERSION); }

  /**
   * @brief Fast Command
   *
   * @return status_t
   */
  inline status_t standby() { return _fastcmd(MCP3x6x_CMD_STANDBY); }

  /**
   * @brief Fast Command
   *
   * @return status_t
   */
  inline status_t shutdown() { return _fastcmd(MCP3x6x_CMD_SHUTDOWN); }

  /**
   * @brief Fast Command
   *
   * @return status_t
   */
  inline status_t full_shutdown() { return _fastcmd(MCP3x6x_CMD_FULL_SHUTDOWN); }

  /**
   * @brief Fast Command
   *
   * @param reset_instance
   * @return status_t
   */
  inline status_t reset(bool reset_instance = false) {
    if (reset_instance) {
      settings.reset();
    }
    return _fastcmd(MCP3x6x_CMD_RESET);
  }

  //  /**
  //   * @brief write settings to ADC
  //   *
  //   * @param data
  //   * @return status_t
  //   */
  //  inline status_t write(MCPSettings::settings_t data) {
  //    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0);
  //  }

  /**
   * @brief write config0 to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::config0_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0);
  }

  /**
   * @brief write config1 to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::config1_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1);
  }

  /**
   * @brief write config2 to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::config2_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2);
  }

  /**
   * @brief write config3 to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::config3_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3);
  }

  /**
   * @brief write IRQ to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::irq_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ);
  }

  /**
   * @brief write MUX to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::mux_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX);
  }

  /**
   * @brief write SCAN to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::scan_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, 3);
  }

  /**
   * @brief write TIMER to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::timer_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_TIMER, 3);
  }

  /**
   * @brief write OFFSET to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::offset_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_OFFSET, 3);
  }

  /**
   * @brief write GAIN to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::gain_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_GAIN, 3);
  }

  /**
   * @brief write LOCK to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::lock_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK);
  }

  /**
   * @brief write CRCCFG to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(MCPSettings::crccfg_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CRCCFG, 2);
  }

  /*
  inline status_t write(MCPSettings::settings_t data) {
    return _transfer((uint8_t)data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, 27);
  }
  */

  /**
   * @brief read register ADCDATA from ADC
   *
   * @param data
   * @return status_t
   */
  status_t read(MCPChannel::Adcdata *data);

  /**
   * @brief read register CONFIG0 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::config0_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0);
  }

  /**
   * @brief read register CONFIG1 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::config1_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1);
  }

  /**
   * @brief read register CONFIG2 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::config2_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2);
  }

  /**
   * @brief read register CONFIG3 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::config3_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3);
  }

  /**
   * @brief read register IRQ from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::irq_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ);
  }

  /**
   * @brief read register MUX from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::mux_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX);
  }

  /**
   * @brief read register SCAN from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::scan_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_SCAN, 3);
  }

  /**
   * @brief read register TIMER from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::timer_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_TIMER, 3);
  }

  /**
   * @brief read register OFFSET from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::offset_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_OFFSET, 3);
  }

  /**
   * @brief read register GAIN from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::gain_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_GAIN, 3);
  }

  /**
   * @brief read register LOCK from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::lock_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK);
  }

  /**
   * @brief read register CRCCFG from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(MCPSettings::crccfg_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CRCCFG, 2);
  }

  /*
  inline status_t read(MCPSettings::settings_t data) {
    return _transfer((uint8_t)data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, 27);
  }
  */

  /**
   * @brief handler
   *
   */
  void IRQ_handler();

  /**
   * @brief does lock the ADC with the given key
   *
   * @param key
   */
  void lock(uint8_t key);

  /**
   * @brief does unlock the ADC
   *
   * @param key
   */
  void unlock();

  /**
   * @brief Set the Data Format
   *
   * @param format
   */
  void setDataFormat(MCPSettings::data_format format);

  /**
   * @brief Set the Conversion Mode
   *
   * @param mode
   */
  void setConversionMode(MCPSettings::conv_mode mode);

  /**
   * @brief Set the Adc Mode object
   *
   * @param mode
   */
  void setAdcMode(MCPSettings::adc_mode mode);

  /**
   * @brief Set the Clock Selection object
   *
   * @param clk
   */
  void setClockSelection(MCPSettings::clk_sel clk);

  /**
   * @brief enable scanning on given channel
   *
   * @param ch
   */
  void enableScanChannel(MCPSettings::mux_t ch);

  /**
   * @brief disable scanning of given channel
   *
   * @param ch
   */
  void disableScanChannel(MCPSettings::mux_t ch);

  /**
   * @brief set oversampling rate
   *
   * @param rate
   */
  void setAveraging(MCPSettings::osr rate);

  /**
   * @brief set gain
   *
   * @param rate
   */
  void setGain(MCPSettings::gain gain);

  /**
   * @brief set resolution to specific amount of bits
   *
   * @param bits
   */
  void setResolution(size_t bits);

  /**
   * @brief set resolution to specific amount of bits
   *
   * @param bits
   */
  void analogReadResolution(size_t bits);

  /**
   * @brief set reference voltage
   *
   * @param vref
   */
  void setReference(float vref = 0.0);

  /**
   * @brief get reference voltage
   *
   * @return float
   */
  float getReference();

  /**
   * @brief get Resolution
   *
   * @return size_t
   */
  size_t getResolution();

  /**
   * @brief get maximal value (=pow(2,resolution))
   *
   * @return uint32_t
   */
  uint32_t getMaxValue();

  /**
   * @brief conversion finished
   *
   * @return true
   * @return false
   */
  bool isComplete();

  /**
   * @brief starts a continuous conversion cycle
   *
   */
  void startContinuous();

  /**
   * @brief stops a continuous conversion cycle
   *
   */
  void stopContinuous();

  /**
   * @brief starts a continuous conversion cycle on differential channels
   *
   */
  void startContinuousDifferential();

  /**
   * @brief checks if continuois mode is set
   *
   * @return true
   * @return false
   */
  bool isContinuous();

  /**
   * @brief set single ended mode
   *
   */
  void singleEndedMode();

  /**
   * @brief set differential mode
   *
   */
  void differentialMode();

  /**
   * @brief returns mode
   *
   * @return true if differential mode is set
   * @return false if single ended mode is set
   */
  bool isDifferential();

  /**
   * @brief returns analog value of given channel
   *
   * @param ch
   * @return int32_t analog value
   */
  int32_t analogRead(MCPSettings::mux_t ch);

  /**
   * @brief read
   *
   * @param ch
   * @return int32_t
   */
  int32_t analogReadContinuous(MCPSettings::mux_t ch);

  /**
   * @brief mux
   *
   * @param pinP
   * @param pinN
   * @return int32_t
   */
  int32_t analogReadDifferential(enum MCPSettings::mux pinP, enum MCPSettings::mux pinN);
};

/**
 * @brief inherited class
 *
 */
class MCP3461 : public MCP3x6x {
 public:
  MCPChannel channel;

  /**
   * @brief Construct a new MCP3461 object
   *
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3461(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel), channel(MCP3461_DEVICE_TYPE){};

  /**
   * @brief Construct a new MCP3461 object
   *
   * @param pinIRQ
   * @param pinMCLK
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3461(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel),
        channel(MCP3461_DEVICE_TYPE){};
};

/**
 * @brief inherited class
 *
 */
class MCP3462 : public MCP3x6x {
 public:
  MCPChannel channel;

  /**
   * @brief Construct a new MCP3462 object
   *
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3462(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel), channel(MCP3461_DEVICE_TYPE){};

  /**
   * @brief Construct a new MCP3462 object
   *
   * @param pinIRQ
   * @param pinMCLK
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3462(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel),
        channel(MCP3461_DEVICE_TYPE){};
};

/**
 * @brief inherited class
 *
 */
class MCP3464 : public MCP3x6x {
 public:
  MCPChannel channel;

  /**
   * @brief Construct a new MCP3464 object
   *
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3464(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel), channel(MCP3461_DEVICE_TYPE){};

  /**
   * @brief Construct a new MCP3464 object
   *
   * @param pinIRQ
   * @param pinMCLK
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3464(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel),
        channel(MCP3461_DEVICE_TYPE){};
};

/**
 * @brief inherited class
 *
 */
class MCP3561 : public MCP3x6x {
 public:
  MCPChannel channel;

  /**
   * @brief Construct a new MCP3561 object
   *
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3561(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel), channel(MCP3461_DEVICE_TYPE){};

  /**
   * @brief Construct a new MCP3561 object
   *
   * @param pinIRQ
   * @param pinMCLK
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3561(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel),
        channel(MCP3461_DEVICE_TYPE){};
};

/**
 * @brief inherited class
 *
 */
class MCP3562 : public MCP3x6x {
 public:
  MCPChannel channel;

  /**
   * @brief Construct a new MCP3562 object
   *
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3562(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel), channel(MCP3461_DEVICE_TYPE){};

  /**
   * @brief Construct a new MCP3562 object
   *
   * @param pinIRQ
   * @param pinMCLK
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3562(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel),
        channel(MCP3461_DEVICE_TYPE){};
};

/**
 * @brief inherited class
 *
 */
class MCP3564 : public MCP3x6x {
 public:
  MCPChannel channel;

  /**
   * @brief Construct a new MCP3564 object
   *
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3564(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel), channel(MCP3461_DEVICE_TYPE){};
  /**
   * @brief Construct a new MCP3564 object
   *
   * @param pinIRQ
   * @param pinMCLK
   * @param pinCS
   * @param theSPI
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3564(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, pinCS, theSPI, pinMOSI, pinMISO, pinCLK, &channel),
        channel(MCP3461_DEVICE_TYPE){};
};

/**
 * @brief ISR wrapper
 *
 */
extern void mcp_wrapper();

#endif  // MCP3x6x_H
