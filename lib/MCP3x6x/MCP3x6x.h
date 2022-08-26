#ifndef MCP3x6x_H
#define MCP3x6x_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <SPI.h>

/* Device IDs */
#define MCP3461_DEVICE_TYPE                (0x0008)
#define MCP3462_DEVICE_TYPE                (0x0009)
#define MCP3464_DEVICE_TYPE                (0x000B)
#define MCP3561_DEVICE_TYPE                (0x000C)
#define MCP3562_DEVICE_TYPE                (0x000D)
#define MCP3564_DEVICE_TYPE                (0x000F)
#define MCP3x6x_DEVICE_ADDRESS             (0x01)
/* SPI Settings */
#define MCP3x6x_SPI_ORDER                  MSBFIRST                       // SPI ORDER
#define MCP3x6x_SPI_MODE                   SPI_MODE0                      // SPI MODE
#define MCP3x6x_SPI_ADR                    (MCP3x6x_DEVICE_ADDRESS << 6)  // SPI ADDRESS
#define MCP3x6x_SPI_SPEED                  (2000000)                      // SPI SPEED Value
/* Fast Commands */
#define MCP3x6x_CMD_CONVERSION             (MCP3x6x_SPI_ADR | 0b101000)
#define MCP3x6x_CMD_STANDBY                (MCP3x6x_SPI_ADR | 0b101100)
#define MCP3x6x_CMD_SHUTDOWN               (MCP3x6x_SPI_ADR | 0b110000)
#define MCP3x6x_CMD_FULL_SHUTDOWN          (MCP3x6x_SPI_ADR | 0b110100)
#define MCP3x6x_CMD_RESET                  (MCP3x6x_SPI_ADR | 0b111000)
#define MCP3x6x_CMD_SREAD                  (MCP3x6x_SPI_ADR | 0b01)
#define MCP3x6x_CMD_IREAD                  (MCP3x6x_SPI_ADR | 0b11)
#define MCP3x6x_CMD_IWRITE                 (MCP3x6x_SPI_ADR | 0b10)
/* Register Addresses */
#define MCP3x6x_ADR_ADCDATA                (MCP3x6x_SPI_ADR | (0x0 << 2))
#define MCP3x6x_ADR_CONFIG0                (MCP3x6x_SPI_ADR | (0x1 << 2))
#define MCP3x6x_ADR_CONFIG1                (MCP3x6x_SPI_ADR | (0x2 << 2))
#define MCP3x6x_ADR_CONFIG2                (MCP3x6x_SPI_ADR | (0x3 << 2))
#define MCP3x6x_ADR_CONFIG3                (MCP3x6x_SPI_ADR | (0x4 << 2))
#define MCP3x6x_ADR_IRQ                    (MCP3x6x_SPI_ADR | (0x5 << 2))
#define MCP3x6x_ADR_MUX                    (MCP3x6x_SPI_ADR | (0x6 << 2))
#define MCP3x6x_ADR_SCAN                   (MCP3x6x_SPI_ADR | (0x7 << 2))
#define MCP3x6x_ADR_TIMER                  (MCP3x6x_SPI_ADR | (0x8 << 2))
#define MCP3x6x_ADR_OFFSETCAL              (MCP3x6x_SPI_ADR | (0x9 << 2))
#define MCP3x6x_ADR_GAINCAL                (MCP3x6x_SPI_ADR | (0xA << 2))
#define MCP3x6x_ADR_RESERVED1              (MCP3x6x_SPI_ADR | (0xB << 2))
#define MCP3x6x_ADR_RESERVED2              (MCP3x6x_SPI_ADR | (0xC << 2))
#define MCP3x6x_ADR_LOCK                   (MCP3x6x_SPI_ADR | (0xD << 2))
#define MCP3x6x_ADR_RESERVED3              (MCP3x6x_SPI_ADR | (0xE << 2))
#define MCP3x6x_ADR_CRCCFG                 (MCP3x6x_SPI_ADR | (0xF << 2))
/* Modi */
#define MCP3x6x_MODE_CONTINUOUS_CONVERSION 3
#define MCP3x6x_MODE_ONESHOT_STANDBY       2
#define MCP3x6x_MODE_ONESHOT_SHUTDOWN      0

/* Register Default Values */
static const uint8_t MCP3x6x_DEFAULT_CONFIG0     = 0xC0;
static const uint8_t MCP3x6x_DEFAULT_CONFIG1     = 0x0C;
static const uint8_t MCP3x6x_DEFAULT_CONFIG2     = 0x8B;
static const uint8_t MCP3x6x_DEFAULT_CONFIG3     = 0x00;
static const uint8_t MCP3x6x_DEFAULT_IRQ         = 0x73;
static const uint8_t MCP3x6x_DEFAULT_MUX         = 0x01;
static const uint8_t MCP3x6x_DEFAULT_SCAN[]      = {0x00, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_TIMER[]     = {0x00, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_OFFSETCAL[] = {0x00, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_GAINCAL[]   = {0x80, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_RESERVED1[] = {0x90, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_RESERVED2   = 0x50;
static const uint8_t MCP3x6x_DEFAULT_LOCK        = 0xA5;
static uint16_t MCP3x6x_DEFAULT_ID               = 0x0000;  // depends on actual used MCP3x6x
static const uint16_t MCP3x6x_DEFAULT_CRCCFG     = 0x0000;
static const uint8_t MCP3x6x_DEFAULTS[27]        = {MCP3x6x_DEFAULT_CONFIG0,  MCP3x6x_DEFAULT_CONFIG1,      MCP3x6x_DEFAULT_CONFIG2,
                                                    MCP3x6x_DEFAULT_CONFIG3,  MCP3x6x_DEFAULT_IRQ,          MCP3x6x_DEFAULT_MUX,
                                                    *MCP3x6x_DEFAULT_SCAN,    *MCP3x6x_DEFAULT_TIMER,       *MCP3x6x_DEFAULT_OFFSETCAL,
                                                    *MCP3x6x_DEFAULT_GAINCAL, *MCP3x6x_DEFAULT_RESERVED1,   MCP3x6x_DEFAULT_RESERVED2,
                                                    MCP3x6x_DEFAULT_LOCK,     (uint16_t)MCP3x6x_DEFAULT_ID, MCP3x6x_DEFAULT_CRCCFG};
class MCP3x6x {
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

  volatile struct Adcdata {
    int32_t offset;
    int32_t vcm;
    int32_t avdd;
    int32_t temp;
    int32_t diff[4];
    int32_t ch[8];
  } _adcdata;

  uint8_t _resolution;
  uint8_t _channels;

  uint8_t _pinCS;
  SPIClass *_spi;

  status_t _fastcmd(uint8_t cmd);
  status_t _transfer(uint8_t *data, uint8_t addr, size_t size = 1);

 public:
  //  void _irq_handler();

  enum class __attribute__((packed)) adc_mode : unsigned {
    conversion = 3,
    standby    = 2,
    shutdown   = 0  // default
  };

  enum class __attribute__((packed)) cs_sel : unsigned {
    bias15uA = 3,
    bias37uA = 2,  // todo check datasheet
    bias09uA = 1,
    bias0uA  = 0  //default
  };

  enum class __attribute__((packed)) clk_sel : unsigned {
    internal_output = 3,
    internal        = 2,
    external        = 0  // default
  };

  enum class __attribute__((packed)) osr : unsigned {
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

  enum class __attribute__((packed)) pre : unsigned {
    MCLK8 = 3,
    MCLK4 = 2,
    MCLK2 = 1,
    MCLK0 = 0  // default
  };

  enum class __attribute__((packed)) boost : unsigned {
    boost3   = 3,
    boost2   = 2,  // default
    boost066 = 1,
    boost05  = 0
  };

  enum class __attribute__((packed)) gain : unsigned {
    gain64 = 7,
    gain32 = 6,
    gain16 = 5,
    gain8  = 4,
    gain4  = 3,
    gain2  = 2,
    gain1  = 1,  // default
    gain13 = 0
  };

  enum class __attribute__((packed)) conv_mode : unsigned {
    continuous       = 3,
    oneshot_standby  = 2,
    oneshot_shutdown = 0  // default
  };

  enum class __attribute__((packed)) data_format : unsigned {
    id_sgnext_data = 3,
    sgnext_data    = 2,
    sgn_data_zero  = 1,
    sgn_data       = 0  // default
  };

  enum class __attribute__((packed)) crc_format : unsigned {
    crc32 = 1,
    crc16 = 0  // default
  };

  enum class __attribute__((packed)) mux : unsigned {
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

  enum class __attribute__((packed)) delay : unsigned {
    dly_512 = 7,
    dly_256 = 6,
    dly_128 = 5,
    dly_64  = 4,
    dly_32  = 3,
    dly_16  = 2,
    dly_8   = 1,
    dly_0   = 0  // default
  };

  // defines which bit has to be set in scan register
  enum class __attribute__((packed)) channelID : unsigned {
    ch_OFFSET = 15,
    ch_VCM    = 14,
    ch_AVDD   = 13,
    ch_TEMP   = 12,
    ch_DIFF_D = 11,
    ch_DIFF_C = 10,
    ch_DIFF_B = 9,
    ch_DIFF_A = 8,
    ch_SE_7   = 7,
    ch_SE_6   = 6,
    ch_SE_5   = 5,
    ch_SE_4   = 4,
    ch_SE_3   = 3,
    ch_SE_2   = 2,
    ch_SE_1   = 1,
    ch_SE_0   = 0
  };

  union Config0 {
    struct {
      enum adc_mode adc : 2;
      enum cs_sel bias  : 2;
      enum clk_sel clk  : 2;
      uint8_t cfg0      : 2;
    };
    uint8_t raw;
  };

  union Config1 {
    struct {
      uint8_t      : 2;  // reserved
      enum osr osr : 4;
      enum pre pre : 2;
    };
    uint8_t raw;
  };

  union Config2 {
    struct {
      uint8_t          : 2;  // reserved
      bool az_mu       : 1;
      enum gain gain   : 3;
      enum boost boost : 2;
    };
    uint8_t raw;
  };

  union Config3 {
    struct {
      bool en_gaincal              : 1;
      bool en_offcal               : 1;
      bool en_crccom               : 1;
      bool crc_format              : 1;
      enum data_format data_format : 2;
      enum conv_mode conv_mode     : 2;
    };
    uint8_t raw;
  };

  union Irq {
    struct {
      bool EN_STP        : 1;
      bool EN_FASTCMD    : 1;
      uint8_t IRQ_MODE   : 2;
      bool POR_STATUS    : 1;
      bool CRCCFG_STATUS : 1;
      bool DR_STATUS     : 1;
      bool               : 1;  // unimplemented
    };
    uint8_t raw;
  };

  union Mux {
    struct {
      enum mux vin_plus  : 4;
      enum mux vin_minus : 4;
    };
    uint8_t raw;
  };

  union Scan {
    struct {
      enum delay dly : 3;
      bool           : 1;  // reserved
      uint8_t        : 4;  // unimplemented
      union {
        struct {
          bool offset          : 1;
          bool vcm             : 1;
          bool avdd            : 1;
          bool temp            : 1;
          uint8_t differential : 4;
          uint8_t single_ended : 8;
        };
        uint16_t raw;
      } channels;
    };
    uint8_t raw[3];
  };

  union Timer {
    uint8_t raw[3];
  };

  union Offsetcal {
    uint8_t raw[3];
  };

  union Gaincal {
    uint8_t raw[3];
  };

  union Lock {
    uint8_t raw;
  };

  union Id {
    uint8_t raw[2];
  };

  union Crccfg {
    uint8_t raw[2];
  };

  union Settings {
    struct {
      Config0 config0;
      Config1 config1;
      Config2 config2;
      Config3 config3;
      Irq irq;
      Mux mux;
      Scan scan;
      Timer timer;
      Offsetcal offsetcal;
      Gaincal gaincal;
      uint8_t reserverd1[3];
      uint8_t reserverd2;
      Lock lock;
      Id id;
      Crccfg crccfg;
    };
    uint8_t raw[27];
  } settings;

  MCP3x6x(const uint16_t MCP3x6x_DEVICE_TYPE);
  ~MCP3x6x() { end(); };

  bool begin(const uint8_t SPI_CS = SS, SPIClass *theSPI = &SPI, uint8_t SPI_MOSI = MOSI, uint8_t SPI_MISO = MISO,
             uint8_t SPI_CLK = SCK);
  bool begin(const uint8_t pinIRQ_MDAT, const uint8_t pinMCLK, bool MCLK_direction = OUTPUT, uint8_t SPI_CS = SS,
             SPIClass *theSPI = &SPI, uint8_t SPI_MOSI = MOSI, uint8_t SPI_MISO = MISO, uint8_t SPI_CLK = SCK);
  void end() { _spi->end(); }

  /* status */
  inline bool status_dr() { return this->_status.dr; }
  inline bool status_crccfg() { return this->_status.crccfg; }
  inline bool status_por() { return this->_status.por; }
  /* fast commands */
  inline status_t conversion() { return this->_fastcmd(MCP3x6x_CMD_CONVERSION); }
  inline status_t standby() { return this->_fastcmd(MCP3x6x_CMD_STANDBY); }
  inline status_t shutdown() { return this->_fastcmd(MCP3x6x_CMD_SHUTDOWN); }
  inline status_t full_shutdown() { return this->_fastcmd(MCP3x6x_CMD_FULL_SHUTDOWN); }
  inline status_t reset() { return this->_fastcmd(MCP3x6x_CMD_RESET); }
  /* write */
  inline status_t write(Config0 data) { return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0); }
  inline status_t write(Config1 data) { return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1); }
  inline status_t write(Config2 data) { return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2); }
  inline status_t write(Config3 data) { return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3); }
  inline status_t write(Irq data) { return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ); }
  inline status_t write(Mux data) { return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX); }
  inline status_t write(Scan data) { return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, sizeof(Scan)); }
  inline status_t write(Timer data) { return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_TIMER, sizeof(Timer)); }
  inline status_t write(Offsetcal data) { return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_OFFSETCAL, sizeof(Offsetcal)); }
  inline status_t write(Gaincal data) { return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_GAINCAL, sizeof(Gaincal)); }
  inline status_t write(Lock data) { return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK); }
  inline status_t write(Crccfg data) { return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CRCCFG, sizeof(Crccfg)); }
  inline status_t write(Settings data) { return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, 27); }
  /* read */
  //  inline status_t read(Adcdata data) { return _transfer(&data, MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, 4); }
  inline status_t read(Config0 data) { return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0); }
  inline status_t read(Config1 data) { return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1); }
  inline status_t read(Config2 data) { return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2); }
  inline status_t read(Config3 data) { return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3); }
  inline status_t read(Irq data) { return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ); }
  inline status_t read(Mux data) { return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX); }
  inline status_t read(Scan data) { return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_SCAN, sizeof(Scan)); }
  inline status_t read(Timer data) { return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_TIMER, sizeof(Timer)); }
  inline status_t read(Offsetcal data) { return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_OFFSETCAL, sizeof(Offsetcal)); }
  inline status_t read(Gaincal data) { return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_GAINCAL, sizeof(Gaincal)); }
  inline status_t read(Lock data) { return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK); }
  inline status_t read(Crccfg data) { return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CRCCFG, sizeof(Crccfg)); }
  inline status_t read(Settings data) { return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, 27); }

  int32_t analogRead(uint8_t channel);
  //  Scan channel2scan(channelID ChannelMask);
  //  Mux channel2mux(channelID ChannelMask);
  void enableScanMode(uint16_t mask);
  void enableMuxMode(uint16_t mask);
  //  void setMode(adc_mode, conv_mode);
  void setMode(uint8_t mode);
};

class MCP3461 : public MCP3x6x {
 public:
  MCP3461() : MCP3x6x(MCP3461_DEVICE_TYPE){};
};

class MCP3462 : public MCP3x6x {
 public:
  MCP3462() : MCP3x6x(MCP3462_DEVICE_TYPE){};
};

class MCP3464 : public MCP3x6x {
 public:
  MCP3464() : MCP3x6x(MCP3464_DEVICE_TYPE){};
};

class MCP3561 : public MCP3x6x {
 public:
  MCP3561() : MCP3x6x(MCP3561_DEVICE_TYPE){};
};

class MCP3562 : public MCP3x6x {
 public:
  MCP3562() : MCP3x6x(MCP3562_DEVICE_TYPE){};
};

class MCP3564 : public MCP3x6x {
 public:
  MCP3564() : MCP3x6x(MCP3564_DEVICE_TYPE){};
};

#endif  // MCP3x6x_H