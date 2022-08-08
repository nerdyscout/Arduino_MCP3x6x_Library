#ifndef MCP3x6x_H
#define MCP3x6x_H

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <SPI.h>

#ifndef BUILD_BUG_ON
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))
#endif

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
#define MCP3x6x_SPI_ORDER     MSBFIRST                       // SPI ORDER
#define MCP3x6x_SPI_MODE      SPI_MODE0                      // SPI MODE
#define MCP3x6x_SPI_ADR       (MCP3x6x_DEVICE_ADDRESS << 6)  // SPI ADDRESS
#define MCP3x6x_SPI_SPEED_MAX (20 * 1000 * 1000)             // SPI SPEED Value
#ifdef MCP3x6x_DEBUG
#define MCP3x6x_SPI_SPEED (MCP3x6x_SPI_SPEED_MAX / 100)
#warning "SPI speed reduced by x100 in debug mode"
#else
#if (__SAMD21__)
#define MCP3x6x_SPI_SPEED (12 * 1000 * 1000)
#warning "SPI speed limited to 12MHz"
#else
#define MCP3x6x_SPI_SPEED (MCP3x6x_SPI_SPEED_MAX)
#endif
#endif

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
static const uint8_t MCP3x6x_DEFAULT_RESERVED2   = 0x30;  // changed with Revision
static const uint8_t MCP3x6x_DEFAULT_LOCK        = 0xA5;
static const uint16_t MCP3x6x_DEFAULT_CRCCFG     = 0x0000;
static const uint8_t MCP3x6x_DEFAULTS[28]        = {
           MCP3x6x_DEFAULT_CONFIG0,  MCP3x6x_DEFAULT_CONFIG1,    MCP3x6x_DEFAULT_CONFIG2,
           MCP3x6x_DEFAULT_CONFIG3,  MCP3x6x_DEFAULT_IRQ,        MCP3x6x_DEFAULT_MUX,
           *MCP3x6x_DEFAULT_SCAN,    *MCP3x6x_DEFAULT_TIMER,     *MCP3x6x_DEFAULT_OFFSETCAL,
           *MCP3x6x_DEFAULT_GAINCAL, *MCP3x6x_DEFAULT_RESERVED1, MCP3x6x_DEFAULT_RESERVED2,
           MCP3x6x_DEFAULT_LOCK,     (uint16_t)0x0000,           MCP3x6x_DEFAULT_CRCCFG};

typedef union {
  struct {
    struct __attribute__((__packed__)) {
      bool por;
      bool crccfg;
      bool dr;
    };
    uint8_t : 3;  // MCP3x6x_DEVICE_ADDRESS
    uint8_t : 2;  // EMTPY
  };
  uint8_t raw;
} status_t;
//BUILD_BUG_ON(sizeof(status_t) != 1);

extern status_t _write(uint8_t *data, uint8_t addr, size_t size = 1);
extern status_t _read(uint8_t *data, uint8_t addr, size_t size = 1);

class MCPSettings {
  enum class __attribute__((packed)) adc_mode {
    conversion = 3,
    standby    = 2,
    shutdown   = 0  // default
  };

  enum class __attribute__((packed)) cs_sel {
    bias15uA = 3,
    bias37uA = 2,  // todo check datasheet
    bias09uA = 1,
    bias0uA  = 0  //default
  };

  enum class __attribute__((packed)) clk_sel {
    internal_output = 3,
    internal        = 2,
    external        = 0  // default
  };

  enum class __attribute__((packed)) osr {
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

  enum class __attribute__((packed)) pre {
    MCLK8 = 3,
    MCLK4 = 2,
    MCLK2 = 1,
    MCLK0 = 0  // default
  };

  enum class __attribute__((packed)) boost {
    boost3   = 3,
    boost2   = 2,  // default
    boost066 = 1,
    boost05  = 0
  };

  enum class __attribute__((packed)) gain {
    gain64 = 7,
    gain32 = 6,
    gain16 = 5,
    gain8  = 4,
    gain4  = 3,
    gain2  = 2,
    gain1  = 1,  // default
    gain13 = 0
  };

  enum class __attribute__((packed)) conv_mode {
    continuous       = 3,
    oneshot_standby  = 2,
    oneshot_shutdown = 0  // default
  };

  enum class __attribute__((packed)) data_format {
    id_sgnext_data = 3,
    sgnext_data    = 2,
    sgn_data_zero  = 1,
    sgn_data       = 0  // default
  };

  enum class __attribute__((packed)) crc_format {
    crc32 = 1,
    crc16 = 0  // default
  };

  enum class __attribute__((packed)) mux {
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

  enum class __attribute__((packed)) delay {
    dly_512 = 7,
    dly_256 = 6,
    dly_128 = 5,
    dly_64  = 4,
    dly_32  = 3,
    dly_16  = 2,
    dly_8   = 1,
    dly_0   = 0  // default
  };

  enum class __attribute__((packed)) channelID {
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

  struct config0 {
    enum adc_mode adc : 2;
    enum cs_sel bias  : 2;
    enum clk_sel clk  : 2;
    uint8_t cfg0      : 2;
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0); };
  };

  struct config1 {
    uint8_t      : 2;  // reserved
    enum osr osr : 4;
    enum pre pre : 2;
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1); };
  };

  struct config2 {
    uint8_t          : 2;  // reserved
    bool az_mu       : 1;
    enum gain gain   : 3;
    enum boost boost : 2;
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2); };
  };

  struct config3 {
    bool en_gaincal              : 1;
    bool en_offcal               : 1;
    bool en_crccom               : 1;
    bool crc_format              : 1;
    enum data_format data_format : 2;
    enum conv_mode conv_mode     : 2;
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3); };
  };

  struct irq {
    bool EN_STP        : 1;
    bool EN_FASTCMD    : 1;
    uint8_t IRQ_MODE   : 2;
    bool POR_STATUS    : 1;
    bool CRCCFG_STATUS : 1;
    bool DR_STATUS     : 1;
    bool               : 1;  // unimplemented
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ); };
  };

  struct mux_s {
    enum mux vin_plus  : 4;
    enum mux vin_minus : 4;
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX); };
  };

  struct scan {
    enum delay dly       : 3;
    bool                 : 1;  // reserved
    uint8_t              : 4;  // unimplemented
    bool offset          : 1;
    bool vcm             : 1;
    bool avdd            : 1;
    bool temp            : 1;
    uint8_t scan_diff_ch : 4;
    uint8_t scan_se_ch   : 8;
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_SCAN); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN); };
  };

  struct timer {
    uint8_t timer[3];
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_TIMER); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_TIMER); };
  };

  struct offsetcal {
    uint8_t offsetcal[3];
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_OFFSETCAL); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_OFFSETCAL); };
  };

  struct gaincal {
    uint8_t gaincal[3];
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_GAINCAL); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_GAINCAL); };
  };

  struct lock {
    uint8_t lock;
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK); };
  };

  struct crccfg {
    uint8_t crccfg[2];
    inline void read() { _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CRCCFG); };
    inline void write() { _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CRCCFG); };
  };

 public:
  //  MCPSettings();
  //  ~MCPSettings();

  struct {
    struct config0 config0;
    struct config1 config1;
    struct config2 config2;
    struct config3 config3;
    struct irq irq;
    struct mux_s mux;
    struct scan scan;
    struct timer timer;
    struct offsetcal offsetcal;
    struct gaincal gaincal;
    uint8_t reserverd1[3];
    uint8_t reserverd2;
    struct lock lock;
    uint16_t id;
    struct crccfg crccfg;
  };

  inline status_t read() {
    return _read((uint8_t *)this, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, 28);
  };
  inline status_t write() {
    return _write((uint8_t *)this, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, 28);
  };
};

class MCP3x6x {
  // device properties
  //  bool _synced      = false;  // true if settings actually written to MCP3x6x

  uint8_t _resolution;
  uint8_t _channels;
  uint16_t _id;

  uint8_t _pinIRQ;
  uint8_t _pinMCLK;
  uint8_t _pinCS;

  SPIClass *_spi;
  status_t _status;

  status_t _write(uint8_t *data, uint8_t addr = MCP3x6x_SPI_ADR, size_t size = 1);
  status_t _read(uint8_t *data, uint8_t addr = MCP3x6x_SPI_ADR, size_t size = 1);

 public:
  MCP3x6x(const uint16_t MCP3x6x_DEVICE_TYPE);
  ~MCP3x6x();

  MCPSettings settings = MCPSettings();

  bool begin(const uint8_t pinCS = SS, SPIClass *theSPI = &SPI);
  bool begin(const uint8_t pinIRQ, const uint8_t pinMCLK, const uint8_t pinCS = SS,
             SPIClass *theSPI = &SPI);
  //  bool available();
  void end();

  inline bool status_dr() { return this->_status.dr; }
  inline bool status_crccfg() { return this->_status.crccfg; }
  inline bool status_por() { return this->_status.por; }

  inline status_t conversion() { return this->_write(MCP3x6x_CMD_CONVERSION); }
  inline status_t standby() { return this->_write(MCP3x6x_CMD_STANDBY); }
  inline status_t shutdown() { return this->_write(MCP3x6x_CMD_SHUTDOWN); }
  inline status_t full_shutdown() { return this->_write(MCP3x6x_CMD_FULL_SHUTDOWN); }
  inline status_t reset() { return this->_write(MCP3x6x_CMD_RESET); }

  int32_t analogRead(uint8_t ch);
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
