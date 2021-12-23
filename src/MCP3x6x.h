#ifndef MCP3x6x_H
#define MCP3x6x_H

#define MCP3461_DEVICE_TYPE (0x0008)
#define MCP3462_DEVICE_TYPE (0x0009)
#define MCP3464_DEVICE_TYPE (0x000B)
#define MCP3561_DEVICE_TYPE (0x000C)
#define MCP3562_DEVICE_TYPE (0x000D)
#define MCP3564_DEVICE_TYPE (0x000F)

#define MCP3x6x_DEBUG          (DEBUG)
#define MCP3x6x_DEVICE_ADDRESS (0x01)
//#define MCP3x6x_DEVICE_TYPE             MCP3564_DEVICE_TYPE

/* SPI Settings */
#define MCP3x6x_SPI_SPEED_MAX (20000000)  // SPI SPEED Value
#ifdef MCP3x6x_DEBUG
#define MCP3x6x_SPI_SPEED (MCP3x6x_SPI_SPEED_MAX / 1000)
#warning "SPI speed reduced by x1000 in debug mode"
#else
#define MCP3x6x_SPI_SPEED (MCP3x6x_SPI_SPEED)
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

/* Register Values */
// config0
#define MCP3x6x_CONFIG0_CLK_SEL_INT_AMCLK (0x03)
#define MCP3x6x_CONFIG0_CLK_SEL_INT       (0x02)
#define MCP3x6x_CONFIG0_CLK_SEL_EXT       (0x00)  // default
#define MCP3x6x_CONFIG0_CS_SEL_15uA       (0x03)
#define MCP3x6x_CONFIG0_CS_SEL_3_7uA      (0x02)
#define MCP3x6x_CONFIG0_CS_SEL_0_9uA      (0x01)
#define MCP3x6x_CONFIG0_CS_SEL_NONE       (0x00)  // default
#define MCP3x6x_CONFIG0_ADC_MODE_CONV     (0x03)
#define MCP3x6x_CONFIG0_ADC_MODE_STANDBY  (0x02)
#define MCP3x6x_CONFIG0_ADC_MODE_OFF      (0x00)  // default
#define MCP3x6x_CONFIG0_DEFAULT                                               \
  {                                                                           \
    (MCP3x6x_CONFIG0_CLK_SEL_EXT << 4) | (MCP3x6x_CONFIG0_CS_SEL_NONE << 2) | \
        MCP3x6x_CONFIG0_ADC_MODE_OFF                                          \
  }
// config1
#define MCP3x6x_CONFIG1_PRE_DIV8  (0x03)
#define MCP3x6x_CONFIG1_PRE_DIV4  (0x02)
#define MCP3x6x_CONFIG1_PRE_DIV2  (0x01)
#define MCP3x6x_CONFIG1_PRE_DIV0  (0x00)  // default
#define MCP3x6x_CONFIG1_OSR_98304 (0x0F)
#define MCP3x6x_CONFIG1_OSR_81920 (0x0E)
#define MCP3x6x_CONFIG1_OSR_49152 (0x0D)
#define MCP3x6x_CONFIG1_OSR_40960 (0x0C)
#define MCP3x6x_CONFIG1_OSR_24576 (0x0B)
#define MCP3x6x_CONFIG1_OSR_20480 (0x0A)
#define MCP3x6x_CONFIG1_OSR_16384 (0x09)
#define MCP3x6x_CONFIG1_OSR_8192  (0x08)
#define MCP3x6x_CONFIG1_OSR_4096  (0x07)
#define MCP3x6x_CONFIG1_OSR_2048  (0x06)
#define MCP3x6x_CONFIG1_OSR_1024  (0x05)
#define MCP3x6x_CONFIG1_OSR_512   (0x04)
#define MCP3x6x_CONFIG1_OSR_256   (0x03)  // default
#define MCP3x6x_CONFIG1_OSR_128   (0x02)
#define MCP3x6x_CONFIG1_OSR_64    (0x01)
#define MCP3x6x_CONFIG1_OSR_32    (0x00)
#define MCP3x6x_CONFIG1_DEFAULT \
  { (MCP3x6x_CONFIG1_PRE_DIV0 << 6) | (MCP3x6x_CONFIG1_OSR_256 << 2) }
// config2
#define MCP3x6x_CONFIG2_BOOST_x2    (0x03)
#define MCP3x6x_CONFIG2_BOOST_x1    (0x02)  // default
#define MCP3x6x_CONFIG2_BOOST_2DIV3 (0x01)
#define MCP3x6x_CONFIG2_BOOST_DIV2  (0x00)
#define MCP3x6x_CONFIG2_GAIN_x64    (0x07)
#define MCP3x6x_CONFIG2_GAIN_x32    (0x06)
#define MCP3x6x_CONFIG2_GAIN_x16    (0x05)
#define MCP3x6x_CONFIG2_GAIN_x8     (0x04)
#define MCP3x6x_CONFIG2_GAIN_x4     (0x03)
#define MCP3x6x_CONFIG2_GAIN_x2     (0x02)
#define MCP3x6x_CONFIG2_GAIN_x1     (0x01)  // default
#define MCP3x6x_CONFIG2_GAIN_DIV3   (0x00)
#define MCP3x6x_CONFIG2_AZ_MUX_ON   (0x01)
#define MCP3x6x_CONFIG2_AZ_MUX_OFF  (0x00)  // default
#define MCP3x6x_CONFIG2_DEFAULT                                        \
  {                                                                    \
    (MCP3x6x_CONFIG2_BOOST_x1 << 6) | (MCP3x6x_CONFIG2_GAIN_x1 << 3) | \
        (MCP3x6x_CONFIG2_AZ_MUX_OFF << 2)                              \
  }
// config3
#define MCP3x6x_CONFIG3_CONV_MODE_CONTINUOUS       (0x03)
#define MCP3x6x_CONFIG3_CONV_MODE_ONE_SHOT_STANDBY (0x02)
#define MCP3x6x_CONFIG3_CONV_MODE_ONE_SHOT_OFF     (0x00)
#define MCP3x6x_CONFIG3_DATA_FORMAT_32BIT_CHID_SGN \
  (0x03)  // right justified data + channel ID + sign extension
#define MCP3x6x_CONFIG3_DATA_FORMAT_32BIT_SGN (0x02)  // right justified data + sign extension
#define MCP3x6x_CONFIG3_DATA_FORMAT_32BIT     (0x01)  // left justified data
#define MCP3x6x_CONFIG3_DATA_FORMAT_24BIT     (0x00)  // default
#define MCP3x6x_CONFIG3_CRC_FORMAT_32         (0x01)
#define MCP3x6x_CONFIG3_CRC_FORMAT_16         (0x00)  // default
#define MCP3x6x_CONFIG3_CRCCOM_ON             (0x01)
#define MCP3x6x_CONFIG3_CRCCOM_OFF            (0x00)  // default
#define MCP3x6x_CONFIG3_OFFCAL_ON             (0x01)
#define MCP3x6x_CONFIG3_OFFCAL_OFF            (0x00)  // default
#define MCP3x6x_CONFIG3_GAINCAL_ON            (0x01)
#define MCP3x6x_CONFIG3_GAINCAL_OFF           (0x00)  // default
#define MCP3x6x_CONFIG3_DEFAULT                                                                \
  {                                                                                            \
    (MCP3x6x_CONFIG3_CONV_MODE_ONE_SHOT_OFF << 6) | (MCP3x6x_CONFIG3_DATA_FORMAT_24BIT << 4) | \
        (MCP3x6x_CONFIG3_CRC_FORMAT_16 << 3) | (MCP3x6x_CONFIG3_CRCCOM_OFF << 2) |             \
        (MCP3x6x_CONFIG3_OFFCAL_OFF << 1) | (MCP3x6x_CONFIG3_GAINCAL_OFF)                      \
  }
// IRQ
#define MCP3x6x_IRQ_MODE_MDAT_HIGHZ (0x02)
#define MCP3x6x_IRQ_MODE_MDAT_HIGH  (0x03)
#define MCP3x6x_IRQ_MODE_IRQ_HIGH   (0x01)
#define MCP3x6x_IRQ_MODE_IRQ_HIGHZ  (0x00)  // default
#define MCP3x6x_IRQ_FASTCMD_ON      (0x01)  // default
#define MCP3x6x_IRQ_FASTCMD_OFF     (0x00)
#define MCP3x6x_IRQ_STP_ON          (0x01)  // default
#define MCP3x6x_IRQ_STP_OFF         (0x00)
#define MCP3x6x_IRQ_DEFAULT \
  { (MCP3x6x_IRQ_MODE_IRQ_HIGHZ << 2) | (MCP3x6x_IRQ_FASTCMD_ON << 1) | (MCP3x6x_IRQ_STP_ON) }
// mux
#define MCP3x6x_MUX_CH_IntVcm    (0xF)
#define MCP3x6x_MUX_CH_IntTemp_M (0xE)
#define MCP3x6x_MUX_CH_IntTemp_P (0xD)
#define MCP3x6x_MUX_CH_REFIN_n   (0xC)
#define MCP3x6x_MUX_CH_REFIN_p   (0xB)
#define MCP3x6x_MUX_CH_AVDD      (0x9)
#define MCP3x6x_MUX_CH_AGND      (0x8)
#define MCP3x6x_MUX_CH7          (0x7)
#define MCP3x6x_MUX_CH6          (0x6)
#define MCP3x6x_MUX_CH5          (0x5)
#define MCP3x6x_MUX_CH4          (0x4)
#define MCP3x6x_MUX_CH3          (0x3)
#define MCP3x6x_MUX_CH2          (0x2)
#define MCP3x6x_MUX_CH1          (0x1)  // default for VIN-
#define MCP3x6x_MUX_CH0          (0x0)  // default for VIN+
#define MCP3x6x_MUX_DEFAULT \
  { (MCP3x6x_MUX_CH0 << 4) | (MCP3x6x_MUX_CH1) }

// scan
#define MCP3x6x_SCAN_DLY_512  (0x000007)
#define MCP3x6x_SCAN_DLY_256  (0x000006)
#define MCP3x6x_SCAN_DLY_128  (0x000005)
#define MCP3x6x_SCAN_DLY_64   (0x000004)
#define MCP3x6x_SCAN_DLY_32   (0x000003)
#define MCP3x6x_SCAN_DLY_16   (0x000002)
#define MCP3x6x_SCAN_DLY_8    (0x000001)
#define MCP3x6x_SCAN_DLY_NONE (0x000000)  // default
#define MCP3x6x_SCAN_DEFAULT \
  { (MCP3x6x_SCAN_DLY_NONE) }  // todo

/* MCP3x6x class */
template <uint16_t MCP3x6x_DEVICE_TYPE>
class MCP3x6x {
 public:
  // channel
  enum class MCP3x6x_Channel : uint8_t {
    SE_0   = 0x00,
    SE_1   = 0x01,
    SE_2   = 0x02,
    SE_3   = 0x03,
    SE_4   = 0x04,
    SE_5   = 0x05,
    SE_6   = 0x06,
    SE_7   = 0x07,
    DIFF_A = 0x08,
    DIFF_B = 0x09,
    DIFF_C = 0x0A,
    DIFF_D = 0x0B,
    TEMP   = 0x0C,  // Internal temperature sensor
    AVDD   = 0x0D,
    VCM    = 0x0E,
    OFFSET = 0x0F
  };

  // status of each SPI communication
  typedef union {
    uint8_t data;
    struct {
      struct {
        bool por;
        bool crccfg;
        bool dr;
      };
      uint8_t ADDR  : 3;
      uint8_t EMTPY : 2;
    };
  } status_t;

  /* MCP registers */
  union config0 {
    uint8_t data;
    struct {
      uint8_t ADC_MODE : 2;
      uint8_t CS_SEL   : 2;
      uint8_t CLK_SEL  : 2;
      uint8_t VREF_SEL : 1;
      uint8_t CONFIG0  : 1;
    };
    inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0); };
    inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0); };
  };

  union config1

  {
    uint8_t data;
    struct {
      uint8_t RESERVED : 2;
      uint8_t OSR      : 4;
      uint8_t PRE      : 2;
    };
    inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1); };
    inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1); };
  };

  union config2 {
    uint8_t data;
    struct {
      uint8_t RESERVED : 1;
      uint8_t AZ_REF   : 1;
      uint8_t AZ_MUX   : 1;
      uint8_t GAIN     : 3;
      uint8_t BOOST    : 2;
    };
    inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2); };
    inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2); };
  };

  union config3 {
    uint8_t data;
    struct {
      uint8_t EN_GAINCAL  : 1;
      uint8_t EN_OFFCAL   : 1;
      uint8_t EN_CRCCOM   : 1;
      uint8_t CRC_FORMAT  : 1;
      uint8_t DATA_FORMAT : 2;
      uint8_t CONV_MODE   : 2;
    };
    inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3); };
    inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3); };
  };

  union irq {
    uint8_t data;
    struct {
      uint8_t EN_STP        : 1;
      uint8_t EN_FASTCMD    : 1;
      uint8_t IRQ_MODE      : 2;
      uint8_t POR_STATUS    : 1;
      uint8_t CRCCFG_STATUS : 1;
      uint8_t DR_STATUS     : 1;
      uint8_t UNIMPLEMENTED : 1;
    };
    inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ); };
    inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ); };
  };

  union mux {
    uint8_t data;
    struct {
      uint8_t MUX_VINM : 4;
      uint8_t MUX_VINP : 4;
    };
    inline status_t write() { return _write(data, (MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX)); };
    inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX); };
  };

  union scan {
    uint8_t data[3];
    struct {
      uint8_t DLY           : 3;
      uint8_t RESERVED      : 1;
      uint8_t UNIMPLEMENTED : 4;
      uint16_t SCAN         : 16;
    };
    inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, 3); };
    inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_SCAN, 3); };
  };

  union lock {
    uint16_t data;
    inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, 2); };
    inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK, 2); };
  };

  /* combine all registers to settings */
  union {
    struct {
      union config0 config0 = MCP3x6x_CONFIG0_DEFAULT;
      union config1 config1 = MCP3x6x_CONFIG1_DEFAULT;
      union config2 config2 = MCP3x6x_CONFIG2_DEFAULT;
      union config3 config3 = MCP3x6x_CONFIG3_DEFAULT;
      union irq irq         = MCP3x6x_IRQ_DEFAULT;
      union mux mux         = MCP3x6x_MUX_DEFAULT;
      union scan scan       = MCP3x6x_SCAN_DEFAULT;
      uint8_t TIMER[3]      = {0x00, 0x00, 0x00};
      uint8_t OFFSETCAL[3];
      uint8_t GAINCAL[3]         = {0x80, 0x00, 0x00};
      const uint8_t RESERVED1[3] = {0x90, 0x00, 0x00};
      const uint8_t RESERVED2    = 0x50;
      uint16_t lock              = 0x0000;
      const uint16_t id          = MCP3x6x_DEVICE_TYPE;
      uint16_t crccfg;
    };
    uint8_t data[28];
    inline status_t write() { return _write(data, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, 28); };
    inline status_t read() { return _read(data, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, 28); };
  } settings;

  // Constructors
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
    //  if (this->settings.config0.CLK_SEL & MASK)
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

  // write Fast Commands
  inline status_t conversion() { return _write(MCP3x6x_CMD_CONVERSION); };
  inline status_t standby() { return _write(MCP3x6x_CMD_STANDBY); };
  inline status_t shutdown() { return _write(MCP3x6x_CMD_SHUTDOWN); };
  inline status_t full_shutdown() { return _write(MCP3x6x_CMD_FULL_SHUTDOWN); };
  inline status_t reset() { return _write(MCP3x6x_CMD_RESET); };
  // get status
  inline bool status_dr() { return this->_status.dr; };
  inline bool status_crccfg() { return this->_status.crccfg; };
  inline bool status_por() { return this->_status.por; };

  // library methodes
  bool begin() {
    reset();
    // settings.read();
    // _resolutionBits =
    return true;
  }

  int32_t analogRead(uint8_t ch) {
    uint8_t buffer[4];
    int32_t adcdata = 0;

    //    this->_selectMultiplexer(ch);
    conversion();

    switch (this->settings.config3.DATA_FORMAT) {
      case MCP3x6x_CONFIG3_CONV_MODE_ONE_SHOT_OFF:
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

 private:
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

    if (this->_hwSPI) {
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

    this->_status.data = s;
    return this->_status;
  }

  status_t _read(uint8_t *data, uint8_t addr, size_t size = 1) {
    uint8_t s = 0;

    if (this->_hwSPI) {
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

    this->_status.data = s;
    return this->_status;
  }

  status_t _selectMultiplexer(uint8_t mux);
};

typedef MCP3x6x<MCP3461_DEVICE_TYPE> MCP3461;
typedef MCP3x6x<MCP3462_DEVICE_TYPE> MCP3462;
typedef MCP3x6x<MCP3464_DEVICE_TYPE> MCP3464;
typedef MCP3x6x<MCP3561_DEVICE_TYPE> MCP3561;
typedef MCP3x6x<MCP3562_DEVICE_TYPE> MCP3562;
typedef MCP3x6x<MCP3564_DEVICE_TYPE> MCP3564;

#endif  // MCP3x6x_H
