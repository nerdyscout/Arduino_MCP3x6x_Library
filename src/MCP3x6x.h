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
static const uint8_t MCP3x6x_DEFAULT_CONFIG1     = 0x00;
static const uint8_t MCP3x6x_DEFAULT_CONFIG2     = 0x00;
static const uint8_t MCP3x6x_DEFAULT_CONFIG3     = 0x00;
static const uint8_t MCP3x6x_DEFAULT_IRQ         = 0x00;
static const uint8_t MCP3x6x_DEFAULT_MUX         = 0x00;
static const uint8_t MCP3x6x_DEFAULT_SCAN[]      = {0x00, 0x00, 0x03};
static const uint8_t MCP3x6x_DEFAULT_TIMER[]     = {0x00, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_OFFSETCAL[] = {0x00, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_GAINCAL[]   = {0x80, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_RESERVED1[] = {0x90, 0x00, 0x00};
static const uint8_t MCP3x6x_DEFAULT_RESERVED2   = 0x50;
static const uint16_t MCP3x6x_DEFAULT_LOCK       = 0x0000;

template <uint16_t MCP3x6x_DEVICE_TYPE>
class MCP3x6x {
 public:
  /*
   enum  crc_format_e : uint8_t {
    crc32 = 1,
    crc16 = 0  // default
  };

  enum  channelID_e : uint8_t {
    ch_OFFSET = 15,
    ch_VCM    = 14,
    ch_AVDD   = 13,
    ch_TEMP   = 12,
    ch_DIFF_D = 11,
    ch_DIFF_C = 10,
    ch_DIFF_B = 9,
    ch_DIFF_A = 9,
    ch_SE_7   = 7,
    ch_SE_6   = 6,
    ch_SE_5   = 5,
    ch_SE_4   = 4,
    ch_SE_3   = 3,
    ch_SE_2   = 2,
    ch_SE_1   = 1,
    ch_SE_0   = 0
  };
*/
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

   private:
    uint8_t _data;
  } status_t;

  //__attribute__((packed))

  typedef union {
    enum mode_e {
      conversion = 3,
      standby    = 2,
      shutdown   = 0  // default
    };

    enum bias_e {
      bias15uA = 3,
      bias37uA = 2,
      bias09uA = 1,
      bias0uA  = 0  //default
    };

    enum clk_e {
      internal_output = 3,
      internal        = 2,
      external        = 0  // default
    };

    struct {
      mode_e adc      : 2;
      bias_e bias     : 2;
      clk_e clk       : 2;
      uint8_t config0 : 2;
    } config0 = MCP3x6x_DEFAULT_CONFIG0;

    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0); };

   private:
    uint8_t _data;
  } config0_t;

  typedef union {
    enum osr_e {
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

    enum pre_e {
      MCLK8 = 3,
      MCLK4 = 2,
      MCLK2 = 1,
      MCLK0 = 0  // default
    };

    struct {
      uint8_t   : 2;
      osr_e osr : 4;
      pre_e pre : 2;
    } config1 = MCP3x6x_DEFAULT_CONFIG1;
    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1); };

   private:
    uint8_t _data;
  } config1_t;

  typedef union {
    enum boost_e {
      boost3   = 3,
      boost2   = 2,  // default
      boost066 = 1,
      boost05  = 0
    };

    enum gain_e {
      gain64 = 7,
      gain32 = 6,
      gain16 = 5,
      gain8  = 4,
      gain4  = 3,
      gain2  = 2,
      gain1  = 1,  // default
      gain13 = 0
    };

    struct {
      uint8_t       : 2;
      bool az_mu    : 1;
      gain_e gain   : 3;
      boost_e boost : 2;
    } config2 = MCP3x6x_DEFAULT_CONFIG2;
    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2); };

   private:
    uint8_t _data;
  } config2_t;

  typedef union {
    enum conv_e {
      continuous       = 3,
      oneshot_standby  = 2,
      oneshot_shutdown = 0  // default
    };

    enum format_e {
      // todo MCP346x vs MCP356x
      id_sgn_24bit   = 3,
      sgn8_24bit     = 2,
      sgn_23bit_zero = 1,
      sgn_23bit      = 0  // default
    };

    struct {
      bool en_gaincal      : 1;
      bool en_offcal       : 1;
      bool en_crccom       : 1;
      bool crc_format      : 1;
      format_e data_format : 2;
      conv_e conv_mode     : 2;
    } config3 = MCP3x6x_DEFAULT_CONFIG3;
    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3); };

   private:
    uint8_t _data;
  } config3_t;

  typedef union {
    struct {
      bool EN_STP        : 1;
      bool EN_FASTCMD    : 1;
      uint8_t IRQ_MODE   : 2;
      bool POR_STATUS    : 1;
      bool CRCCFG_STATUS : 1;
      bool DR_STATUS     : 1;
      bool               : 1;
    } irq = MCP3x6x_DEFAULT_IRQ;
    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ); };

   private:
    uint8_t _data;
  } irq_t;

  typedef union {
    enum mux_e {
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

    struct {
      mux_e vin_plus  : 4;
      mux_e vin_minus : 4;
    } mux = MCP3x6x_DEFAULT_MUX;
    inline status_t write() { return _write(_data, (MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX)); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX); };

   private:
    uint8_t _data;
  } mux_t;

  typedef union {
    enum delay_e {
      dly_512 = 7,
      dly_256 = 6,
      dly_128 = 5,
      dly_64  = 4,
      dly_32  = 3,
      dly_16  = 2,
      dly_8   = 1,
      dly_0   = 0  // default
    };

    struct {
      delay_e dly : 3;
      bool        : 1;
      uint8_t     : 4;
      uint8_t scan[2];
    } scan = MCP3x6x_DEFAULT_SCAN;
    inline status_t write() {
      return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, sizeof(_data));
    };
    inline status_t read() {
      return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_SCAN, sizeof(_data));
    };

   private:
    uint8_t _data[3];
  } scan_t;

  union lock {
    uint16_t lock = MCP3x6x_DEFAULT_LOCK;

    inline status_t write() {
      return _write(lock, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, sizeof(lock));
    };
    inline status_t read() {
      return _read(lock, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK, sizeof(lock));
    };
  };

  union registers {
    struct {
      config0_t config0;
      config1_t config1;
      config2_t config2;
      config3_t config3;
      irq_t irq;
      mux_t mux;
      scan_t scan;
      uint8_t timer[3];
      uint8_t offsetcal[3];
      uint8_t gaincal[3];
      uint8_t reserverd1[3];
      uint8_t reserverd2;
      uint16_t lock;
      uint16_t id;
      uint16_t crccfg;
    };

    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, 28); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, 28); };

   private:
    uint8_t _data[28];
  };

  static constexpr union registers defaults = {
      MCP3x6x_DEFAULT_CONFIG0, MCP3x6x_DEFAULT_CONFIG1,   MCP3x6x_DEFAULT_CONFIG2,
      MCP3x6x_DEFAULT_CONFIG3, MCP3x6x_DEFAULT_IRQ,       MCP3x6x_DEFAULT_MUX,
      MCP3x6x_DEFAULT_SCAN,    MCP3x6x_DEFAULT_TIMER,     MCP3x6x_DEFAULT_OFFSETCAL,
      MCP3x6x_DEFAULT_GAINCAL, MCP3x6x_DEFAULT_RESERVED1, MCP3x6x_DEFAULT_RESERVED2,
      MCP3x6x_DEFAULT_LOCK,    MCP3x6x_DEVICE_TYPE,       0x0000};
  union registers settings = defaults;

  MCP3x6x(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI) {
    _hwSPI = true;

    _pinCS = pinCS;

    pinMode(_pinCS, OUTPUT);
    digitalWrite(_pinCS, HIGH);

    _spi = theSPI;
    _spi->begin();
  }

  MCP3x6x(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
          SPIClass *theSPI = &SPI) {
    _hwSPI   = true;

    _pinCS   = pinCS;
    _pinIRQ  = pinIRQ;
    _pinMCLK = pinMCLK;

    pinMode(_pinCS, OUTPUT);
    pinMode(_pinIRQ, INPUT);
    pinMode(_pinMCLK, OUTPUT);  // TODDO: this pin changes depending on CLK_SEK

    digitalWrite(_pinCS, HIGH);

    _spi = theSPI;
    _spi->begin();
  }

  MCP3x6x(const uint8_t pinCS, const uint8_t pinSCK = SCK, const uint8_t pinMOSI = MOSI,
          const uint8_t pinMISO = MISO) {
    _hwSPI   = false;

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
    _hwSPI   = false;

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
    if (_hwSPI) _spi->end();
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
    this->reset();

    this->settings.read();

    //    this->settings.read();
    /*
    if (_pinMCLK == NULL) {
      this->settings.config0.clk_sel = clk_sel::internal;
      this->settings.config0.write();
    }
*/

    // _resolutionBits =
    return true;
  }

  int32_t analogRead(uint8_t ch) {
    uint8_t buffer[4];
    int32_t adcdata = 0;

    //    _selectMultiplexer(ch);
    this->conversion();

    switch (this->settings.config3) {
      // todo MCP346x vs MCP356x
      case MCP3x6x::config3_t::sgn_23bit:
        _status = _read(buffer, (MCP3x6x_CMD_IREAD | MCP3x6x_ADR_ADCDATA), 3);
        break;
      default:
        _status = _read(buffer, (MCP3x6x_CMD_IREAD | MCP3x6x_ADR_ADCDATA), 4);
    }

    memcpy(&buffer, &adcdata, 3);
    return adcdata;
  };
  //  int32_t analogReadDifferential(uint8_t inPositiveChannel) const
  //  void analogWriteResolution(uint8_t bits);
  //  void analogReadResolution(uint8_t bits);
  //  bool available()
  /*
  class analogread : Stream() {
    analogRead();
    ~analogRead() { reset(); }
  }
*/
 private:
  //  MCP3x6x(const MCP3x6x &);
  //  MCP3x6x &operator=(const MCP3x6x &);

  // SPI
  SPIClass *_spi;
  bool _hwSPI;
  //    bool _CSkeepAlive=false; // TODO
  uint8_t _pinCS;
  uint8_t _pinMOSI;
  uint8_t _pinMISO;
  uint8_t _pinSCK;
  uint8_t _pinIRQ;
  uint8_t _pinMCLK;

  // device properties
  status_t _status;
  uint8_t _resolutionBits;
  uint16_t _channelMask;

  // SPI communication
  status_t _write(uint8_t *data, uint8_t addr = MCP3x6x_SPI_ADR, size_t size = 1) {
    uint8_t s = 0;

    if (_hwSPI) {
      _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
      digitalWrite(_pinCS, LOW);
      s = _spi->transfer(addr);
      for (uint8_t i = 0; i < size - 1; i++) {
        _spi->transfer(data[i]);
      }
      digitalWrite(_pinCS, HIGH);
      _spi->endTransaction();

    } else {
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
    }

    this->_status._data = s;
    return _status;
  }

  status_t _read(uint8_t *data, uint8_t addr, size_t size = 1) {
    uint8_t s = 0;

    if (_hwSPI) {
      _spi->beginTransaction(SPISettings(MCP3x6x_SPI_SPEED, MCP3x6x_SPI_ORDER, MCP3x6x_SPI_MODE));
      digitalWrite(_pinCS, LOW);
      s = _spi->transfer(addr);
      for (uint8_t i = 0; i < size; i++) {
        data[i] = _spi->transfer(0x00);
      }
      digitalWrite(_pinCS, HIGH);
      _spi->endTransaction();

    } else {
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
    }

    _status->_data = s;
    return _status;
  }

  //  status_t _selectMultiplexer(uint8_t mux);
};

typedef MCP3x6x<MCP3461_DEVICE_TYPE> MCP3461;
typedef MCP3x6x<MCP3462_DEVICE_TYPE> MCP3462;
typedef MCP3x6x<MCP3464_DEVICE_TYPE> MCP3464;
typedef MCP3x6x<MCP3561_DEVICE_TYPE> MCP3561;
typedef MCP3x6x<MCP3562_DEVICE_TYPE> MCP3562;
typedef MCP3x6x<MCP3564_DEVICE_TYPE> MCP3564;

#endif  // MCP3x6x_H
