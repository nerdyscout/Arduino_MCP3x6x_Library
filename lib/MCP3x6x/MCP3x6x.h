// SPDX-License-Identifier: MIT

#ifndef MCP3x6x_H
#define MCP3x6x_H

#define MCP3x6x_DEBUG DEBUG

#if ARDUINO >= 100
#  include "Arduino.h"
#else
#  include "WProgram.h"
#endif

#include <SPI.h>

#define MCP_OFFSET                (0x88)
#define MCP_VCM                   (0xF8)
#define MCP_AVDD                  (0x98)
#define MCP_TEMP                  (0xDE)
#define MCP_DIFFD                 (0x67)
#define MCP_DIFFC                 (0x45)
#define MCP_DIFFB                 (0x23)
#define MCP_DIFFA                 (0x01)
#define MCP_CH7                   (0x78)
#define MCP_CH6                   (0x68)
#define MCP_CH5                   (0x58)
#define MCP_CH4                   (0x48)
#define MCP_CH3                   (0x38)
#define MCP_CH2                   (0x28)
#define MCP_CH1                   (0x18)
#define MCP_CH0                   (0x08)
/* Device IDs */
#define MCP3461_DEVICE_TYPE       (0x0008)
#define MCP3462_DEVICE_TYPE       (0x0009)
#define MCP3464_DEVICE_TYPE       (0x000B)
#define MCP3561_DEVICE_TYPE       (0x000C)
#define MCP3562_DEVICE_TYPE       (0x000D)
#define MCP3564_DEVICE_TYPE       (0x000F)
#define MCP3x6x_DEVICE_ADDRESS    (0x01)
/* SPI Settings */
#define MCP3x6x_SPI_ORDER         MSBFIRST                       // SPI ORDER
#define MCP3x6x_SPI_MODE          SPI_MODE0                      // SPI MODE
#define MCP3x6x_SPI_ADR           (MCP3x6x_DEVICE_ADDRESS << 6)  // SPI ADDRESS
#define MCP3x6x_SPI_SPEED         (200000)                       // SPI SPEED Value
/* Fast Commands */
#define MCP3x6x_CMD_CONVERSION    (MCP3x6x_SPI_ADR | 0b101000)
#define MCP3x6x_CMD_STANDBY       (MCP3x6x_SPI_ADR | 0b101100)
#define MCP3x6x_CMD_SHUTDOWN      (MCP3x6x_SPI_ADR | 0b110000)
#define MCP3x6x_CMD_FULL_SHUTDOWN (MCP3x6x_SPI_ADR | 0b110100)
#define MCP3x6x_CMD_RESET         (MCP3x6x_SPI_ADR | 0b111000)
#define MCP3x6x_CMD_SREAD         (MCP3x6x_SPI_ADR | 0b01)
#define MCP3x6x_CMD_IREAD         (MCP3x6x_SPI_ADR | 0b11)
#define MCP3x6x_CMD_IWRITE        (MCP3x6x_SPI_ADR | 0b10)
/* Register Addresses */
#define MCP3x6x_ADR_ADCDATA       (MCP3x6x_SPI_ADR | (0x0 << 2))
#define MCP3x6x_ADR_CONFIG0       (MCP3x6x_SPI_ADR | (0x1 << 2))
#define MCP3x6x_ADR_CONFIG1       (MCP3x6x_SPI_ADR | (0x2 << 2))
#define MCP3x6x_ADR_CONFIG2       (MCP3x6x_SPI_ADR | (0x3 << 2))
#define MCP3x6x_ADR_CONFIG3       (MCP3x6x_SPI_ADR | (0x4 << 2))
#define MCP3x6x_ADR_IRQ           (MCP3x6x_SPI_ADR | (0x5 << 2))
#define MCP3x6x_ADR_MUX           (MCP3x6x_SPI_ADR | (0x6 << 2))
#define MCP3x6x_ADR_SCAN          (MCP3x6x_SPI_ADR | (0x7 << 2))
#define MCP3x6x_ADR_TIMER         (MCP3x6x_SPI_ADR | (0x8 << 2))
#define MCP3x6x_ADR_OFFSET        (MCP3x6x_SPI_ADR | (0x9 << 2))
#define MCP3x6x_ADR_GAIN          (MCP3x6x_SPI_ADR | (0xA << 2))
#define MCP3x6x_ADR_RESERVED1     (MCP3x6x_SPI_ADR | (0xB << 2))
#define MCP3x6x_ADR_RESERVED2     (MCP3x6x_SPI_ADR | (0xC << 2))
#define MCP3x6x_ADR_LOCK          (MCP3x6x_SPI_ADR | (0xD << 2))
#define MCP3x6x_ADR_RESERVED3     (MCP3x6x_SPI_ADR | (0xE << 2))
#define MCP3x6x_ADR_CRCCFG        (MCP3x6x_SPI_ADR | (0xF << 2))

class MCP3x6x {
  /* Register Default Values */
  const uint8_t _DEFAULT_CONFIG0      = 0xC0;
  const uint8_t _DEFAULT_CONFIG1      = 0x0C;
  const uint8_t _DEFAULT_CONFIG2      = 0x8B;
  const uint8_t _DEFAULT_CONFIG3      = 0x00;
  const uint8_t _DEFAULT_IRQ          = 0x73;
  const uint8_t _DEFAULT_MUX          = 0x01;
  const uint8_t _DEFAULT_SCAN[3]      = {0x00, 0x00, 0x00};
  const uint8_t _DEFAULT_TIMER[3]     = {0x00, 0x00, 0x00};
  const uint8_t _DEFAULT_OFFSET[3]    = {0x00, 0x00, 0x00};
  const uint8_t _DEFAULT_GAIN[3]      = {0x80, 0x00, 0x00};
  const uint8_t _DEFAULT_RESERVED1[3] = {0x90, 0x00, 0x00};
  const uint8_t _DEFAULT_RESERVED2    = 0x50;
  const uint8_t _DEFAULT_LOCK         = 0xA5;
  const uint8_t _DEFAULT_CRCCFG[2]    = {0x00, 0x00};
  const uint8_t _DEFAULTS[27]         = {_DEFAULT_CONFIG0, _DEFAULT_CONFIG1,    _DEFAULT_CONFIG2,
                                         _DEFAULT_CONFIG3, _DEFAULT_IRQ,        _DEFAULT_MUX,
                                         *_DEFAULT_SCAN,   *_DEFAULT_TIMER,     *_DEFAULT_OFFSET,
                                         *_DEFAULT_GAIN,   *_DEFAULT_RESERVED1, _DEFAULT_RESERVED2,
                                         _DEFAULT_LOCK,    (uint16_t)0x0000,    *_DEFAULT_CRCCFG};

  typedef union {
    struct {
      struct __attribute__((__packed__)) {
        bool por;
        bool crccfg;
        bool dr;
      };
      uint8_t      : 1;  // !addr[0]
      uint8_t addr : 2;
      uint8_t      : 2;  // EMTPY
    };
    uint8_t raw;
  } status_t;
  status_t _status;

  status_t _fastcmd(uint8_t cmd) { return _transfer(0x00, cmd, 0); }
  void _reverse_array(uint8_t *array, size_t size);
  status_t _transfer(uint8_t *data, uint8_t addr, size_t size = 1);
  int32_t _getValue(uint32_t raw);
  uint8_t _getChannel(uint32_t raw);

  SPIClass *_spi;
  uint8_t _pinMISO, _pinMOSI, _pinCLK, _pinCS;
  uint8_t _pinMCLK, _pinIRQ;

  bool _differential = false;
  float _reference   = 3.3;
  size_t _resolution, _resolution_max;
  size_t _channels_max;
  uint16_t _channel_mask;
  const uint8_t _channelID[16] = {MCP_CH0,  MCP_CH1,  MCP_CH2,   MCP_CH3,   MCP_CH4,   MCP_CH5,
                                  MCP_CH6,  MCP_CH7,  MCP_DIFFA, MCP_DIFFB, MCP_DIFFC, MCP_DIFFD,
                                  MCP_TEMP, MCP_AVDD, MCP_VCM,   MCP_OFFSET};

 public:
  enum class __attribute__((packed)) adc_mode : uint8_t {
    conversion = 3,
    standby    = 2,
    shutdown   = 0  // default
  };

  enum class __attribute__((packed)) cs_sel : uint8_t {
    bias15uA = 3,
    bias37uA = 2,  // todo check datasheet
    bias09uA = 1,
    bias0uA  = 0  //default
  };

  enum class __attribute__((packed)) clk_sel : uint8_t {
    internal_output = 3,
    internal        = 2,
    external        = 0  // default
  };

  enum class __attribute__((packed)) osr : uint8_t {
    OSR98304 = 15,
    OSR81920 = 14,
    OSR49152 = 13,
    OSR40960 = 12,
    OSR24576 = 11,
    OSR20480 = 10,
    OSR16384 = 9,
    OSR8192  = 8,
    OSR4096  = 7,
    OSR2048  = 6,
    OSR1024  = 5,
    OSR512   = 4,
    OSR256   = 3,  // default
    OSR128   = 2,
    OSR64    = 1,
    OSR32    = 0
  };

  enum class __attribute__((packed)) pre : uint8_t {
    MCLK8 = 3,
    MCLK4 = 2,
    MCLK2 = 1,
    MCLK0 = 0  // default
  };

  enum class __attribute__((packed)) boost : uint8_t {
    boost3   = 3,
    boost2   = 2,  // default
    boost066 = 1,
    boost05  = 0
  };

  enum class __attribute__((packed)) gain : uint8_t {
    gain64 = 7,
    gain32 = 6,
    gain16 = 5,
    gain8  = 4,
    gain4  = 3,
    gain2  = 2,
    gain1  = 1,  // default
    gain13 = 0
  };

  enum class __attribute__((packed)) conv_mode : uint8_t {
    continuous       = 3,
    oneshot_standby  = 2,
    oneshot_shutdown = 0  // default
  };

  enum class __attribute__((packed)) data_format : uint8_t {
    id_sgnext_data = 3,
    sgnext_data    = 2,
    sgn_data_zero  = 1,
    sgn_data       = 0  // default
  };

  enum class __attribute__((packed)) crc_format : uint8_t {
    crc32 = 1,
    crc16 = 0  // default
  };

  enum class __attribute__((packed)) mux : uint8_t {
    mux_VCM          = 15,
    mux_TemperatureM = 14,
    mux_TemperatureP = 13,
    mux_REFINM       = 12,
    mux_REFINP       = 11,
    mux_AVDD         = 9,
    mux_AGND         = 8,
    mux_CH7          = 7,
    mux_CH6          = 6,
    mux_CH5          = 5,
    mux_CH4          = 4,
    mux_CH3          = 3,
    mux_CH2          = 2,
    mux_CH1          = 1,  // default vin-
    mux_CH0          = 0   // default vin+
  };

  enum class __attribute__((packed)) delay : uint8_t {
    dly_512 = 7,
    dly_256 = 6,
    dly_128 = 5,
    dly_64  = 4,
    dly_32  = 3,
    dly_16  = 2,
    dly_8   = 1,
    dly_0   = 0  // default
  };

  struct Adcdata {
    uint8_t channelid : 4;
    int32_t value     : 25;
  } adcdata;  // structure with latest read value

  typedef union {
    struct {
      enum adc_mode adc : 2;
      enum cs_sel bias  : 2;
      enum clk_sel clk  : 2;
      bool vref_sel     : 1;
      uint8_t cfg0      : 1;
    };
    uint8_t raw;
  } config0_t;

  typedef union {
    struct {
      uint8_t      : 2;  // reserved
      enum osr osr : 4;
      enum pre pre : 2;
    };
    uint8_t raw;
  } config1_t;

  typedef union {
    struct {
      uint8_t          : 2;  // reserved
      bool az_mu       : 1;
      enum gain gain   : 3;
      enum boost boost : 2;
    };
    uint8_t raw;
  } config2_t;

  typedef union {
    struct {
      bool en_gaincal              : 1;
      bool en_offcal               : 1;
      bool en_crccom               : 1;
      bool crc_format              : 1;
      enum data_format data_format : 2;
      enum conv_mode conv_mode     : 2;
    };
    uint8_t raw;
  } config3_t;

  typedef union {
    struct {
      bool en_stp        : 1;
      bool en_fastcmd    : 1;
      uint8_t irq_mode   : 2;
      bool por_status    : 1;
      bool crccfg_status : 1;
      bool dr_status     : 1;
      bool               : 1;  // unimplemented
    };
    uint8_t raw;
  } irq_t;

  typedef union Mux {
    struct {
      enum mux vin_minus : 4;
      enum mux vin_plus  : 4;
    };
    uint8_t raw;
    Mux(uint8_t data) : raw(data){};
  } mux_t;

  typedef union {
    struct {
      union {
        struct {
          uint8_t single_ended : 8;
          uint8_t differential : 4;
          bool temp            : 1;
          bool avdd            : 1;
          bool vcm             : 1;
          bool offset          : 1;
        };
        uint16_t raw;
      } channel;
      uint8_t        : 4;  // unimplemented
      bool           : 1;  // reserved
      enum delay dly : 3;
    };
    uint8_t raw[3];
  } scan_t;

  typedef union {
    uint8_t raw[3];
  } timer_t;

  typedef union {
    uint8_t raw[3];
  } offset_t;

  typedef union {
    uint8_t raw[3];
  } gain_t;

  typedef union {
    uint8_t raw;
  } lock_t;

  typedef union {
    uint8_t raw[2];
  } crccfg_t;

  struct Settings {
    config0_t config0;
    config1_t config1;
    config2_t config2;
    config3_t config3;
    irq_t irq;
    mux_t mux;
    scan_t scan;
    timer_t timer;
    offset_t offsetcal;
    gain_t gaincal;
    uint8_t reserverd1[3];
    uint8_t reserverd2;
    lock_t lock;
    uint16_t id;
    crccfg_t crccfg;
    Settings() : mux(0x01){};
  } settings;

  union {
    struct {
      int32_t ch[8];
      int32_t diff[4];
      int32_t temp;
      int32_t avdd;
      int32_t vcm;
      int32_t offset;
    };
    uint32_t raw[16];
  } result;  // structure with latest value per channel

  uint8_t channels;

  MCP3x6x(const uint16_t MCP3x6x_DEVICE_TYPE, const uint8_t pinCS, SPIClass *theSPI,
          const uint8_t pinMOSI, const uint8_t pinMISO, const uint8_t pinCLK);
  MCP3x6x(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint16_t MCP3x6x_DEVICE_TYPE,
          const uint8_t pinCS, SPIClass *theSPI, const uint8_t pinMOSI, const uint8_t pinMISO,
          const uint8_t pinCLK);
  ~MCP3x6x() { end(); };

  bool begin(uint16_t channelmask = 0, float vref = 0.0);
  void end() { _spi->end(); }

  /* status */
  inline bool status_dr() { return _status.dr; }
  inline bool status_crccfg() { return _status.crccfg; }
  inline bool status_por() { return _status.por; }

  /* fast commands */
  inline status_t conversion() { return _fastcmd(MCP3x6x_CMD_CONVERSION); }
  inline status_t standby() { return _fastcmd(MCP3x6x_CMD_STANDBY); }
  inline status_t shutdown() { return _fastcmd(MCP3x6x_CMD_SHUTDOWN); }
  inline status_t full_shutdown() { return _fastcmd(MCP3x6x_CMD_FULL_SHUTDOWN); }
  inline status_t reset() {
    //    memcpy(settings, _DEFAULTS, 27);
    return _fastcmd(MCP3x6x_CMD_RESET);
  }

  /* write */
  inline status_t write(config0_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0);
  }
  inline status_t write(config1_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1);
  }
  inline status_t write(config2_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2);
  }
  inline status_t write(config3_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3);
  }
  inline status_t write(irq_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ);
  }
  inline status_t write(mux_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX);
  }
  inline status_t write(scan_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, 3);
  }
  inline status_t write(timer_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_TIMER, 3);
  }
  inline status_t write(offset_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_OFFSET, 3);
  }
  inline status_t write(gain_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_GAIN, 3);
  }
  inline status_t write(lock_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK);
  }
  inline status_t write(crccfg_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CRCCFG, 2);
  }
  /*
  inline status_t write(Settings data) {
    return _transfer((uint8_t)data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, 27);
  }
  */

  /* read */
  status_t read(Adcdata *data);
  inline status_t read(config0_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0);
  }
  inline status_t read(config1_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1);
  }
  inline status_t read(config2_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2);
  }
  inline status_t read(config3_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3);
  }
  inline status_t read(irq_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ);
  }
  inline status_t read(mux_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX);
  }
  inline status_t read(scan_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_SCAN, 3);
  }
  inline status_t read(timer_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_TIMER, 3);
  }
  inline status_t read(offset_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_OFFSET, 3);
  }
  inline status_t read(gain_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_GAIN, 3);
  }
  inline status_t read(lock_t data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK);
  }
  inline status_t read(crccfg_t data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CRCCFG, 2);
  }
  /*
  inline status_t read(Settings data) {
    return _transfer((uint8_t)data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, 27);
  }
  */

  void IRQ_handler();

  void lock(uint8_t key = 0x5A);
  void unlock();

  void setDataFormat(data_format format);
  void setConversionMode(conv_mode mode);
  void setAdcMode(adc_mode mode);
  void setClockSelection(clk_sel clk);
  void enableScanChannel(mux_t ch);
  void disableScanChannel(mux_t ch);
  // ...further functions may follow...

  int32_t analogRead(mux_t ch);
  void setResolution(size_t bits);
  void analogReadResolution(size_t bits);
  void setReference(float vref = 0.0);
  float getReference();
  uint32_t getMaxValue();
  bool isComplete();
  void startContinuous();
  void stopContinuous();
  void startContinuousDifferential();
  bool isContinuous();
  void singleEndedMode();
  void differentialMode();
  bool isDifferential();
  int32_t analogReadContinuous(mux_t ch);
  int32_t analogReadDifferential(mux pinP, mux pinN);
};

class MCP3461 : public MCP3x6x {
 public:
  MCP3461(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(MCP3461_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
  MCP3461(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, MCP3461_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
};

class MCP3462 : public MCP3x6x {
 public:
  MCP3462(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(MCP3462_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
  MCP3462(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, MCP3462_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
};

class MCP3464 : public MCP3x6x {
 public:
  MCP3464(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(MCP3464_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
  MCP3464(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, MCP3464_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
};
class MCP3561 : public MCP3x6x {
 public:
  MCP3561(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(MCP3561_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
  MCP3561(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, MCP3561_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
};

class MCP3562 : public MCP3x6x {
 public:
  MCP3562(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(MCP3562_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
  MCP3562(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, MCP3562_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
};

class MCP3564 : public MCP3x6x {
 public:
  MCP3564(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO, const uint8_t pinCLK = SCK)
      : MCP3x6x(MCP3564_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
  MCP3564(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO,
          const uint8_t pinCLK = SCK)
      : MCP3x6x(pinIRQ, pinMCLK, MCP3564_DEVICE_TYPE, pinCS, theSPI, pinMOSI, pinMISO, pinCLK){};
};

extern void mcp_wrapper();

#endif  // MCP3x6x_H
