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
#define MCP3x6x_SPI_SPEED (MCP3x6x_SPI_SPEED_MAX / 1000)
#warning "SPI speed reduced by x1000 in debug mode"
#else
#define MCP3x6x_SPI_SPEED (MCP3x6x_SPI_SPEED_MAX)
#endif
#define MCP3x6x_SPI_ORDER MSBFIRST                       // SPI ORDER
#define MCP3x6x_SPI_MODE  SPI_MODE0                      // SPI MODE
#define MCP3x6x_SPI_ADR   (MCP3x6x_DEVICE_ADDRESS << 6)  // SPI ADDRESS

/* Fast Commands */
#define MCP3x6x_CMD_CONVERSION    (byte *)(MCP3x6x_SPI_ADR | 0b101000)
#define MCP3x6x_CMD_STANDBY       (byte *)(MCP3x6x_SPI_ADR | 0b101100)
#define MCP3x6x_CMD_SHUTDOWN      (byte *)(MCP3x6x_SPI_ADR | 0b110000)
#define MCP3x6x_CMD_FULL_SHUTDOWN (byte *)(MCP3x6x_SPI_ADR | 0b110100)
#define MCP3x6x_CMD_RESET         (byte *)(MCP3x6x_SPI_ADR | 0b111000)
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

template <uint16_t MCP3x6x_DEVICE_TYPE>
class MCP3x6x {
 public:
  enum class clk_sel : uint8_t {
    internal_output = 3,
    internal        = 2,
    external        = 0  // default
  };

  enum class cs_sel : uint8_t {
    bias15uA = 3,
    bias37uA = 2,
    bias09uA = 1,
    bias0uA  = 0  //default
  };

  enum class adc_mode : uint8_t {
    conversion_mode = 3,
    standby_mode    = 2,
    shutdown_mode   = 0  // default
  };

  enum class pre : uint8_t {
    MCLK8 = 3,
    MCLK4 = 2,
    MCLK2 = 1,
    MCLK0 = 0  // default
  };

  enum class osr : uint8_t {
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
    OSR32    = 0,
  };

  enum class boost : uint8_t {
    boost2   = 3,
    boost    = 2,  // default
    boost066 = 1,
    boost05  = 0
  };

  enum class gain : uint8_t {
    gain64 = 7,
    gain32 = 6,
    gain16 = 5,
    gain8  = 4,
    gain4  = 3,
    gain2  = 2,
    gain1  = 1,  // default
    gain13 = 0
  };

  enum class conv_mode : uint8_t {
    continuous       = 3,
    oneshot_standby  = 2,
    oneshot_shutdown = 0  // default
  };

  enum class data_format : uint8_t {
    id_sgn_bit24   = 3,
    sgn8_bit24     = 2,
    sgn_bit23_zero = 1,
    sgn_bit23      = 0  // default
  };

  enum class crc_format : uint8_t {
    crc32 = 1,
    crc16 = 0  // default
  };

  enum class activate : uint8_t { enabled = 1, disabled = 0 };

  enum class mux_vin : uint8_t {
    VCM          = 15,
    TemperatureM = 14,
    TemperatureP = 13,
    REFINM       = 12,
    REFINP       = 11,
    AVDD         = 9,
    AGND         = 8,
    CH7          = 7,
    CH6          = 6,
    CH5          = 5,
    CH4          = 4,
    CH3          = 3,
    CH2          = 2,
    CH1          = 1,  // default vin-
    CH0          = 0   // default vin+
  };

  enum class dly : uint8_t {
    dly_512  = 7,
    dly_256  = 6,
    dly_128  = 5,
    dly_64   = 4,
    dly_32   = 3,
    dly_16   = 2,
    dly_8    = 1,
    dly_none = 0  // default
  };

  enum class channel_ID : uint8_t {
    OFFSET = 15,
    VCM    = 14,
    AVDD   = 13,
    TEMP   = 12,
    DIFF_D = 11,
    DIFF_C = 10,
    DIFF_B = 9,
    DIFF_A = 9,
    SE_7   = 7,
    SE_6   = 6,
    SE_5   = 5,
    SE_4   = 4,
    SE_3   = 3,
    SE_2   = 2,
    SE_1   = 1,
    SE_0   = 0
  };

  typedef union {
    struct {
      struct {
        bool por;
        bool crccfg;
        bool dr;
      };
      uint8_t ADDR  : 3;
      uint8_t EMTPY : 2;
    };

   private:
    uint8_t _data;
  } status_t;

  typedef union {
    struct {
      enum adc_mode adc_mode : 2;
      enum cs_sel cs_sel     : 2;
      enum clk_sel clk_sel   : 2;
      uint8_t config0        : 2;
    };
    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0); };

   private:
    uint8_t _data;
  } config0_t;

  typedef union {
    struct {
      //      uint8_t RESERVED : 2;
      enum osr osr : 4;
      enum pre pre : 2;
    };
    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1); };

   private:
    uint8_t _data;
  } config1_t;

  typedef union {
    struct {
      //      uint8_t RESERVED     : 2;
      enum activate az_mux : 1;
      enum gain gain       : 3;
      enum boost boost     : 2;
    };
    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2); };

   private:
    uint8_t _data;
  } config2_t;

  typedef union {
    struct {
      enum activate en_gaincal     : 1;
      enum activate en_offcal      : 1;
      enum activate en_crccom      : 1;
      enum activate crc_format     : 1;
      enum data_format data_format : 2;
      enum conv_mode conv_mode     : 2;
    };
    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3); };

   private:
    uint8_t _data;
  } config3_t;

  typedef union {
    struct {
      uint8_t EN_STP        : 1;
      uint8_t EN_FASTCMD    : 1;
      uint8_t IRQ_MODE      : 2;
      uint8_t POR_STATUS    : 1;
      uint8_t CRCCFG_STATUS : 1;
      uint8_t DR_STATUS : 1, : 1;
    };
    inline status_t write() { return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ); };

   private:
    uint8_t _data;
  } irq_t;

  typedef union {
    struct {
      enum mux_vin mux_vinp : 4;
      enum mux_vin mux_vinm : 4;
    };
    inline status_t write() { return _write(_data, (MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX)); };
    inline status_t read() { return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX); };

   private:
    uint8_t _data;
  } mux_t;

  typedef union {
    struct {
      enum dly dly : 3, : 1, : 4;
      uint8_t SCAN[2];
    };
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
    inline status_t write() {
      return _write(_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, sizeof(_data));
    };
    inline status_t read() {
      return _read(_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK, sizeof(_data));
    };

   private:
    uint16_t _data;
  };

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
    // _resolutionBits =
    return true;
  }

  int32_t analogRead(uint8_t ch) {
    uint8_t buffer[4];
    int32_t adcdata = 0;

    //    _selectMultiplexer(ch);
    conversion();

    switch (settings.config3.data_format) {
      case data_format::sgn_bit23:
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
  //      MCP3x6x(const MCP3x6x&);
  //      MCP3x6x& operator = (const MCP3x6x&);

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

    _status._data = s;
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

    _status._data = s;
    return _status;
  }

 public:
  union {
    struct {
      config0_t config0;
      config1_t config1;
      config2_t config2;
      config3_t config3;
      irq_t irq;
      mux_t mux;
      scan_t scan;
      uint8_t TIMER[3] = {0x00, 0x00, 0x00};
      uint8_t OFFSETCAL[3];
      uint8_t GAINCAL[3]         = {0x80, 0x00, 0x00};
      const uint8_t RESERVED1[3] = {0x90, 0x00, 0x00};
      const uint8_t RESERVED2    = 0x50;
      uint16_t lock              = 0x0000;
      const uint16_t id          = MCP3x6x_DEVICE_TYPE;
      uint16_t crccfg;
    };
    inline status_t write() {
      return _write(this->_data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, 28);
    };
    inline status_t read() {
      return _read(this->_data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, 28);
    };

   private:
    uint8_t _data[28];

  } settings;

  //  status_t _selectMultiplexer(uint8_t mux);
};

typedef MCP3x6x<MCP3461_DEVICE_TYPE> MCP3461;
typedef MCP3x6x<MCP3462_DEVICE_TYPE> MCP3462;
typedef MCP3x6x<MCP3464_DEVICE_TYPE> MCP3464;
typedef MCP3x6x<MCP3561_DEVICE_TYPE> MCP3561;
typedef MCP3x6x<MCP3562_DEVICE_TYPE> MCP3562;
typedef MCP3x6x<MCP3564_DEVICE_TYPE> MCP3564;

#endif  // MCP3x6x_H
