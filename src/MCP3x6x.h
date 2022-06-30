#ifndef MCP3x6x_H
#define MCP3x6x_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <SPI.h>

#if DEBUG
#define MCP3x6x_DEBUG (DEBUG)
#endif

#define MCP3461_DEVICE_TYPE (0x0008)
#define MCP3462_DEVICE_TYPE (0x0009)
#define MCP3464_DEVICE_TYPE (0x000B)
#define MCP3561_DEVICE_TYPE (0x000C)
#define MCP3562_DEVICE_TYPE (0x000D)
#define MCP3564_DEVICE_TYPE (0x000F)

#define MCP3x6x_DEVICE_ADDRESS (0x01)

/* SPI Settings */
#define MCP3x6x_SPI_SPEED_MAX (20000000)  // SPI SPEED Value
#ifdef MCP3x6x_DEBUG
#define MCP3x6x_SPI_SPEED (MCP3x6x_SPI_SPEED_MAX / 100)
#warning "SPI speed reduced by x100 in debug mode"
#else
#define MCP3x6x_SPI_SPEED (MCP3x6x_SPI_SPEED_MAX)
#endif
#define MCP3x6x_SPI_ORDER MSBFIRST                       // SPI ORDER
#define MCP3x6x_SPI_MODE  SPI_MODE0                      // SPI MODE
#define MCP3x6x_SPI_ADR   (MCP3x6x_DEVICE_ADDRESS << 6)  // SPI ADDRESS

/* Fast Commands */
#define MCP3x6x_CMD_CONVERSION    (uint8_t *)(MCP3x6x_SPI_ADR | 0b101000)
#define MCP3x6x_CMD_STANDBY       (uint8_t *)(MCP3x6x_SPI_ADR | 0b101100)
#define MCP3x6x_CMD_SHUTDOWN      (uint8_t *)(MCP3x6x_SPI_ADR | 0b110000)
#define MCP3x6x_CMD_FULL_SHUTDOWN (uint8_t *)(MCP3x6x_SPI_ADR | 0b110100)
#define MCP3x6x_CMD_RESET         (uint8_t *)(MCP3x6x_SPI_ADR | 0b111000)
#define MCP3x6x_CMD_SREAD         (MCP3x6x_SPI_ADR | 0b01)
#define MCP3x6x_CMD_IREAD         (MCP3x6x_SPI_ADR | 0b11)
#define MCP3x6x_CMD_IWRITE        (MCP3x6x_SPI_ADR | 0b10)

/* Register Addresses */
#define MCP3x6x_ADR_ADCDATA   (MCP3x6x_SPI_ADR | (0x0 << 2))
#define MCP3x6x_ADR_CONFIG0   (MCP3x6x_SPI_ADR | (0x1 << 2))
#define MCP3x6x_ADR_CONFIG1   (MCP3x6x_SPI_ADR | (0x2 << 2))
#define MCP3x6x_ADR_CONFIG2   (MCP3x6x_SPI_ADR | (0x3 << 2))
#define MCP3x6x_ADR_CONFIG3   (MCP3x6x_SPI_ADR | (0x4 << 2))
#define MCP3x6x_ADR_IRQ       (MCP3x6x_SPI_ADR | (0x5 << 2))
#define MCP3x6x_ADR_MUX       (MCP3x6x_SPI_ADR | (0x6 << 2))
#define MCP3x6x_ADR_SCAN      (MCP3x6x_SPI_ADR | (0x7 << 2))
#define MCP3x6x_ADR_TIMER     (MCP3x6x_SPI_ADR | (0x8 << 2))
#define MCP3x6x_ADR_OFFSETCAL (MCP3x6x_SPI_ADR | (0x9 << 2))
#define MCP3x6x_ADR_GAINCAL   (MCP3x6x_SPI_ADR | (0xA << 2))
#define MCP3x6x_ADR_RESERVED1 (MCP3x6x_SPI_ADR | (0xB << 2))
#define MCP3x6x_ADR_RESERVED2 (MCP3x6x_SPI_ADR | (0xC << 2))
#define MCP3x6x_ADR_LOCK      (MCP3x6x_SPI_ADR | (0xD << 2))
#define MCP3x6x_ADR_RESERVED3 (MCP3x6x_SPI_ADR | (0xE << 2))
#define MCP3x6x_ADR_CRCCFG    (MCP3x6x_SPI_ADR | (0xF << 2))

/* Register Default Values */
static const uint8_t MCP3x6x_DEFAULT_CONFIG0     = 0x00;
static const uint8_t MCP3x6x_DEFAULT_CONFIG1     = 0x0C;
static const uint8_t MCP3x6x_DEFAULT_CONFIG2     = 0x8B;
static const uint8_t MCP3x6x_DEFAULT_CONFIG3     = 0x00;
static const uint8_t MCP3x6x_DEFAULT_IRQ         = 0x73;
static const uint8_t MCP3x6x_DEFAULT_MUX         = 0x01;
static const uint8_t MCP3x6x_DEFAULT_SCAN[]      = {0x00, 0x00, 0x03};
static const uint8_t MCP3x6x_DEFAULT_TIMER[]     = {0x00, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_OFFSETCAL[] = {0x00, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_GAINCAL[]   = {0x80, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_RESERVED1[] = {0x90, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_RESERVED2   = 0x50;
static const uint8_t MCP3x6x_DEFAULT_LOCK        = 0xA5;
static const uint16_t MCP3x6x_DEFAULT_CRCCFG     = 0x0000;

template <uint16_t MCP3x6x_DEVICE_TYPE>
class MCP3x6x {
 private:
  //  MCP3x6x(const MCP3x6x &);
  //  MCP3x6x &operator=(const MCP3x6x &);

  typedef union {
    struct {
      struct {
        bool por;
        bool crccfg;
        bool dr;
      } status;
      uint8_t ADDR  : 3;
      uint8_t EMTPY : 2;
    };
    uint8_t raw;
  } status_t;

  // device properties
  static status_t _status;
  enum class _hwMode { BitBang = 0, SPI = 1 } _hwMode;

#if MCP3x6x_DEVICE_TYPE == MCP3461_DEVICE_TYPE
  static const uint8_t _resolutionBits = 16;
  static const uint8_t _channelCount   = 2;
#elif MCP3x6x_DEVICE_TYPE == MCP3462_DEVICE_TYPE
  static const uint8_t _resolutionBits = 16;
  static const uint8_t _channelCount   = 4;
#elif MCP3x6x_DEVICE_TYPE == MCP3464_DEVICE_TYPE
  static const uint8_t _resolutionBits = 16;
  static const uint8_t _channelCount   = 8;
#elif MCP3x6x_DEVICE_TYPE == MCP3561_DEVICE_TYPE
  static const uint8_t _resolutionBits = 24;
  static const uint8_t _channelCount   = 2;
#elif MCP3x6x_DEVICE_TYPE == MCP3562_DEVICE_TYPE
  static const uint8_t _resolutionBits = 24;
  static const uint8_t _channelCount   = 4;
#elif MCP3x6x_DEVICE_TYPE == MCP3564_DEVICE_TYPE
  static const uint8_t _resolutionBits = 24;
  static const uint8_t _channelCount   = 8;
#endif

  // SPI
  SPIClass *_spi;
  uint8_t _pinCS;
  uint8_t _pinMOSI;
  uint8_t _pinMISO;
  uint8_t _pinSCK;
  uint8_t _pinIRQ;
  uint8_t _pinMCLK;

  // SPI communication
  status_t _write(uint8_t *data, uint8_t addr = MCP3x6x_SPI_ADR, size_t size = 1) {
    uint8_t s = 0;

    switch (_hwMode) {
      case _hwMode::SPI:
        _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
        digitalWrite(_pinCS, LOW);
        s = _spi->transfer(addr);
        for (uint8_t i = 0; i < size - 1; i++) {
          _spi->transfer(data[i]);
        }
        digitalWrite(_pinCS, HIGH);
        _spi->endTransaction();
        break;

      default:
      case _hwMode::BitBang:
        digitalWrite(_pinCS, LOW);
        // addr
        for (uint8_t c = 0; c < 8; c++) {
          digitalWrite(_pinMOSI, (addr >> (7 - c)) & 0x01);
          digitalWrite(_pinSCK, HIGH);
          digitalWrite(_pinSCK, LOW);
          s <<= 1;
          if (digitalRead(_pinMISO)) s |= 0x01;
        }
        // data
        for (uint8_t i = 0; i < size - 1; i++) {
          for (int c = 0; c < 8; c++) {
            digitalWrite(_pinMOSI, (data[i] >> (7 - c)) & 0x01);
            digitalWrite(_pinSCK, HIGH);
            digitalWrite(_pinSCK, LOW);
          }
        }
        digitalWrite(_pinCS, HIGH);
        break;
    }

    this->_status.raw = s;
    return this->_status;
  }

  status_t _read(uint8_t *data, uint8_t addr = MCP3x6x_SPI_ADR, size_t size = 1) {
    uint8_t s = 0;

    switch (_hwMode) {
      case _hwMode::SPI:
        _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
        digitalWrite(_pinCS, LOW);
        s = _spi->transfer(addr);
        for (uint8_t i = 0; i < size; i++) {
          data[i] = _spi->transfer(0x00);
        }
        digitalWrite(_pinCS, HIGH);
        _spi->endTransaction();
        break;

      default:
      case _hwMode::BitBang:
        digitalWrite(_pinCS, LOW);
        // addr
        for (uint8_t c = 0; c < 8; c++) {
          digitalWrite(_pinMOSI, (addr >> (7 - c)) & 0x01);
          digitalWrite(_pinSCK, HIGH);
          digitalWrite(_pinSCK, LOW);
          s <<= 1;
          if (digitalRead(_pinMISO)) s |= 0x01;
        }
        // data
        for (uint8_t i = 0; i < size; i++) {
          for (int c = 0; c < 8; c++) {
            digitalWrite(_pinMOSI, (data[i] >> (7 - c)) & 0x01);
            digitalWrite(_pinSCK, HIGH);
            digitalWrite(_pinSCK, LOW);
          }
        }
        digitalWrite(_pinCS, HIGH);
        break;
    }

    this->_status.raw = s;
    return this->_status;
  }

 public:
  enum class mode_e {
    conversion = 3,
    standby    = 2,
    shutdown   = 0  // default
  };

  enum class bias_e {
    bias15uA = 3,
    bias37uA = 2,  // todo check datasheet
    bias09uA = 1,
    bias0uA  = 0  //default
  };

  enum class clk_e {
    internal_output = 3,
    internal        = 2,
    external        = 0  // default
  };

  enum class osr_e {
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

  enum class pre_e {
    MCLK8 = 3,
    MCLK4 = 2,
    MCLK2 = 1,
    MCLK0 = 0  // default
  };

  enum class boost_e {
    boost3   = 3,
    boost2   = 2,  // default
    boost066 = 1,
    boost05  = 0
  };

  enum class gain_e {
    gain64 = 7,
    gain32 = 6,
    gain16 = 5,
    gain8  = 4,
    gain4  = 3,
    gain2  = 2,
    gain1  = 1,  // default
    gain13 = 0
  };

  enum class conv_mode_e {
    continuous       = 3,
    oneshot_standby  = 2,
    oneshot_shutdown = 0  // default
  };

  enum class data_format_e {
    id_sgnext_data = 3,
    sgnext_data    = 2,
    sgn_data_zero  = 1,
    sgn_data       = 0  // default
  };

  enum class crc_format_e {
    crc32 = 1,
    crc16 = 0  // default
  };

  enum class mux_e {
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

  enum class delay_e {
    dly_512 = 7,
    dly_256 = 6,
    dly_128 = 5,
    dly_64  = 4,
    dly_32  = 3,
    dly_16  = 2,
    dly_8   = 1,
    dly_0   = 0  // default
  };

  enum class channelID_e {
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

  // registers
  union config0_u {
    struct {
      mode_e adc   : 2;
      bias_e bias  : 2;
      clk_e clk    : 2;
      uint8_t cfg0 : 2;
    };
    uint8_t raw = MCP3x6x_DEFAULT_CONFIG0;
    // inline status_t write() { return _write(_raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0); };
    // inline status_t read() { return _read(_raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0); };
  };

  union config1_u {
    struct {
      uint8_t   : 2;  // reserved
      osr_e osr : 4;
      pre_e pre : 2;
    };
    uint8_t raw = MCP3x6x_DEFAULT_CONFIG1;
    // inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1); };
    // inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1); };
  };

  union config2_u {
    struct {
      uint8_t       : 2;  // reserved
      bool az_mu    : 1;
      gain_e gain   : 3;
      boost_e boost : 2;
    };
    uint8_t raw = MCP3x6x_DEFAULT_CONFIG2;
    // inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2); };
    // inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2); };
  };

  union config3_u {
    struct {
      bool en_gaincal           : 1;
      bool en_offcal            : 1;
      bool en_crccom            : 1;
      bool crc_format           : 1;
      data_format_e data_format : 2;
      conv_mode_e conv_mode     : 2;
    };
    uint8_t raw = MCP3x6x_DEFAULT_CONFIG3;
    // inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3); };
    // inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3); };
  };

  union irq_u {
    struct {
      bool EN_STP        : 1;
      bool EN_FASTCMD    : 1;
      uint8_t IRQ_MODE   : 2;
      bool POR_STATUS    : 1;
      bool CRCCFG_STATUS : 1;
      bool DR_STATUS     : 1;
      bool               : 1;  // unimplemented
    };
    uint8_t raw = MCP3x6x_DEFAULT_IRQ;
    // inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ); };
    // inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ); };
  };

  union mux_u {
    struct {
      mux_e vin_plus  : 4;
      mux_e vin_minus : 4;
    };
    uint8_t raw = MCP3x6x_DEFAULT_MUX;
    // inline status_t write() { return _write(data, (MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX)); };
    // inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX); };
  };

  union scan_u {
    struct {
      delay_e dly          : 3;
      bool                 : 1;  // reserved
      uint8_t              : 4;  // unimplemented
      bool offset          : 1;
      bool vcm             : 1;
      bool avdd            : 1;
      bool temp            : 1;
      uint8_t scan_diff_ch : 4;
      uint8_t scan_se_ch   : 8;
    };
    uint8_t raw = *MCP3x6x_DEFAULT_SCAN;
    // inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, 3); };
    // inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_SCAN, 3); };
  };

  // inline status_t write() { return _write(lock, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, sizeof(lock));};
  // inline status_t read() {return _read(lock, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK, sizeof(lock));};

  // inline status_t read() {return _read(crc, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CRCCFG, sizeof(crc));};

  union settings {
    struct {
      union config0_u config0;
      union config1_u config1;
      union config2_u config2;
      union config3_u config3;
      union irq_u irq;
      union mux_u mux;
      union scan_u scan;
      uint8_t timer[3];
      uint8_t offsetcal[3];
      uint8_t gaincal[3];
      uint32_t : 24;
      uint8_t  : 8;
      uint8_t lock;
      uint16_t id;
      uint16_t crc;
    };
    uint8_t raw[28] = {
        MCP3x6x_DEFAULT_CONFIG0,  MCP3x6x_DEFAULT_CONFIG1,    MCP3x6x_DEFAULT_CONFIG2,
        MCP3x6x_DEFAULT_CONFIG3,  MCP3x6x_DEFAULT_IRQ,        MCP3x6x_DEFAULT_MUX,
        *MCP3x6x_DEFAULT_SCAN,    *MCP3x6x_DEFAULT_TIMER,     *MCP3x6x_DEFAULT_OFFSETCAL,
        *MCP3x6x_DEFAULT_GAINCAL, *MCP3x6x_DEFAULT_RESERVED1, MCP3x6x_DEFAULT_RESERVED2,
        MCP3x6x_DEFAULT_LOCK,     MCP3x6x_DEVICE_TYPE,        MCP3x6x_DEFAULT_CRCCFG};

    // getter
    inline union settings get() {
      _status = _read(raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, sizeof(raw));
      return *this;
    };

    mode_e get_adc() {
      _status = _read(config0, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0);
      return config0.adc;
    }

    bias_e get_bias() {
      _status = _read(config0, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0);
      return config0.bias;
    }

    clk_e get_clk() {
      _status = _read(config0, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0);
      return config0.clk;
    }

    uint8_t get_cfg0() {
      _status = _read(config0, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0);
      return config0.cfg0;
    }

    // setter
    inline void set() {
      _status = _write(raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, sizeof(raw));
    };

  } settings;

  MCP3x6x(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI) {
    _hwMode = _hwMode::SPI;
    _pinCS  = pinCS;

    pinMode(_pinCS, OUTPUT);
    digitalWrite(_pinCS, HIGH);

    _spi = theSPI;
    _spi->begin();
  }

  MCP3x6x(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI) {
    _hwMode  = _hwMode::SPI;

    _pinCS   = pinCS;
    _pinIRQ  = pinIRQ;
    _pinMCLK = pinMCLK;

    pinMode(_pinCS, OUTPUT);
    pinMode(_pinIRQ, INPUT);
    pinMode(_pinMCLK, OUTPUT);  // TODO: this pin changes depending on CLK_SEK

    digitalWrite(_pinCS, HIGH);

    _spi = theSPI;
    _spi->begin();
  }

  MCP3x6x(const uint8_t pinCS, const uint8_t pinSCK = SCK, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO) {
    _hwMode  = _hwMode::BitBang;

    _pinSCK  = pinSCK;
    _pinMOSI = pinMOSI;
    _pinMISO = pinMISO;
    _pinCS   = pinCS;

    pinMode(_pinSCK, OUTPUT);
    pinMode(_pinMOSI, OUTPUT);
    pinMode(_pinMISO, INPUT);
    pinMode(_pinCS, OUTPUT);

    digitalWrite(_pinSCK, LOW);
    digitalWrite(_pinMOSI, LOW);
    digitalWrite(_pinCS, HIGH);
  }

  MCP3x6x(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          const uint8_t pinSCK = SCK, const uint8_t pinMOSI = MOSI, const uint8_t pinMISO = MISO) {
    _hwMode  = _hwMode::BitBang;

    _pinSCK  = pinSCK;
    _pinMOSI = pinMOSI;
    _pinMISO = pinMISO;
    _pinCS   = pinCS;
    _pinIRQ  = pinIRQ;
    _pinMCLK = pinMCLK;

    pinMode(_pinSCK, OUTPUT);
    pinMode(_pinMOSI, OUTPUT);
    pinMode(_pinMISO, INPUT);
    pinMode(_pinCS, OUTPUT);
    pinMode(_pinIRQ, INPUT);
    //  if (settings.config0.CLK_SEL & MASK)
    //  {
    pinMode(_pinMCLK, OUTPUT);  // TODDO: this pin changes depending on CLK_SEK
                                //  }

    digitalWrite(_pinSCK, LOW);
    digitalWrite(_pinMOSI, LOW);
    digitalWrite(_pinCS, HIGH);
  }

  ~MCP3x6x() {
    if (_hwMode == _hwMode::SPI) _spi->end();
  }

  inline status_t conversion() { return _write(MCP3x6x_CMD_CONVERSION); };
  inline status_t standby() { return _write(MCP3x6x_CMD_STANDBY); };
  inline status_t shutdown() { return _write(MCP3x6x_CMD_SHUTDOWN); };
  inline status_t full_shutdown() { return _write(MCP3x6x_CMD_FULL_SHUTDOWN); };
  inline status_t reset() { return _write(MCP3x6x_CMD_RESET); };

  inline bool status_dr() { return _status.dr; };
  inline bool status_crccfg() { return _status.crccfg; };
  inline bool status_por() { return _status.por; };

  bool begin() {
    reset();

    settings.get();
    if (settings.id == MCP3x6x_DEVICE_TYPE) {
      return true;
    }
    return false;
  }

  bool available(){
    return _spi->initialized ;
  }

  int32_t analogRead(uint8_t ch) {
    uint8_t buffer[4];
    int32_t adcdata = 0;

    //    _selectMultiplexer(ch);
    conversion();

    switch (settings.config3.data_format) {
      case (MCP3x6x::data_format_e::sgn_data):
        _status = _read(buffer, (MCP3x6x_CMD_IREAD | MCP3x6x_ADR_ADCDATA), 3);
        break;
      default:
        _status = _read(buffer, (MCP3x6x_CMD_IREAD | MCP3x6x_ADR_ADCDATA), 4);
    }
    // todo ensure signedness is correct

    memcpy(&buffer, &adcdata, 3);
    return adcdata;
  };

  //  int32_t analogReadDifferential(uint8_t inPositiveChannel) const
  /*
  class analogRead : Stream() {
    analogRead();
    ~analogRead() { reset(); }
  }
*/

  //  status_t selectMultiplexer(uint8_t mux);
};

typedef MCP3x6x<MCP3461_DEVICE_TYPE> MCP3461;
typedef MCP3x6x<MCP3462_DEVICE_TYPE> MCP3462;
typedef MCP3x6x<MCP3464_DEVICE_TYPE> MCP3464;
typedef MCP3x6x<MCP3561_DEVICE_TYPE> MCP3561;
typedef MCP3x6x<MCP3562_DEVICE_TYPE> MCP3562;
typedef MCP3x6x<MCP3564_DEVICE_TYPE> MCP3564;

#endif  // MCP3x6x_H
