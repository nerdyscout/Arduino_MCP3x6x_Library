// SPDX-License-Identifier: MIT

/**
 * @file MCPSetting.h
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.2
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef MCP_SETTINGS_H
#define MCP_SETTINGS_H

#include <Arduino.h>

/**
 * @brief base class settings
 *
 */
class MCPSettings {
  friend class MCP3x6x;
  friend class MCPChannel;

  static const uint8_t _DEFAULT_CONFIG0          = 0xC0;                //!< default value
  static const uint8_t _DEFAULT_CONFIG1          = 0x0C;                //!< default value
  static const uint8_t _DEFAULT_CONFIG2          = 0x8B;                //!< default value
  static const uint8_t _DEFAULT_CONFIG3          = 0x00;                //!< default value
  static const uint8_t _DEFAULT_IRQ              = 0x73;                //!< default value
  static const uint8_t _DEFAULT_MUX              = 0x01;                //!< default value
  static constexpr uint8_t _DEFAULT_SCAN[3]      = {0x00, 0x00, 0x00};  //!< default value
  static constexpr uint8_t _DEFAULT_TIMER[3]     = {0x00, 0x00, 0x00};  //!< default value
  static constexpr uint8_t _DEFAULT_OFFSET[3]    = {0x00, 0x00, 0x00};  //!< default value
  static constexpr uint8_t _DEFAULT_GAIN[3]      = {0x80, 0x00, 0x00};  //!< default value
  static constexpr uint8_t _DEFAULT_RESERVED1[3] = {0x90, 0x00, 0x00};  //!< default value
  static const uint8_t _DEFAULT_RESERVED2        = 0x50;                //!< default value
  static const uint8_t _DEFAULT_LOCK             = 0xA5;                //!< default value
  static constexpr uint8_t _DEFAULT_CRCCFG[2]    = {0x00, 0x00};        //!< default value

 public:
  /**
   * @brief ADC Operating Mode Selection
   *
   */
  enum class __attribute__((packed)) adc_mode : uint8_t {
    CONVERSION = 3,  //!< ADC Conversion mode
    STANDBY    = 2,  //!< ADC Standby mode
    SHUTDOWN   = 0   //!< ADC shutdown mode (default)
  };

  /**
   * @brief Current Source/Sink Selection Bits for Sensor Bias.
   *
   */
  enum class __attribute__((packed)) cs_sel : uint8_t {
    BIAS_15UA = 3,  //!< 15 μA is applied to the ADC inputs
    BIAS_37UA = 2,  //!< 3.7 μA is applied to the ADC inputs
    BIAS_09UA = 1,  //!< 0.9 μA is applied to the ADC inputs
    BIAS_0UA  = 0   //!< No current source is applied to the ADC inputs (default)
  };

  /**
   * @brief clock selection
   *
   */
  enum class __attribute__((packed)) clk_sel : uint8_t {
    INTERN_OUTPUT = 3,  //!< Internal clock is selected and AMCLK is present on the analog master
                        //!< clock output pin
    INTERN = 2,  //!< Internal clock is selected and no clock output is present on the CLK pin
    EXTERN = 0   //!< External digital clock (default)
  };

  /**
   * @brief Oversampling Ratio for Delta-Sigma A/D Conversion
   *
   */
  enum class __attribute__((packed)) osr : uint8_t {
    OSR_98304 = 15,  //!< OSR: 98304
    OSR_81920 = 14,  //!< OSR: 81920
    OSR_49152 = 13,  //!< OSR: 49152
    OSR_40960 = 12,  //!< OSR: 40960
    OSR_24576 = 11,  //!< OSR: 24576
    OSR_20480 = 10,  //!< OSR: 20480
    OSR_16384 = 9,   //!< OSR: 16384
    OSR_8192  = 8,   //!< OSR: 8192
    OSR_4096  = 7,   //!< OSR: 4096
    OSR_2048  = 6,   //!< OSR: 2048
    OSR_1024  = 5,   //!< OSR: 1024
    OSR_512   = 4,   //!< OSR: 512
    OSR_256   = 3,   //!< OSR: 256 (default)
    OSR_128   = 2,   //!< OSR: 128
    OSR_64    = 1,   //!< OSR: 64
    OSR_32    = 0    //!< OSR: 32
  };

  /**
   * @brief Prescaler Value Selection for AMCLK
   *
   */
  enum class __attribute__((packed)) pre : uint8_t {
    MCLK_8 = 3,  //!< AMCLK = MCLK/8
    MCLK_4 = 2,  //!< AMCLK = MCLK/4
    MCLK_2 = 1,  //!< AMCLK = MCLK/2
    MCLK_0 = 0   //!< AMCLK = MCLK (default)
  };

  /**
   * @brief ADC Bias Current Selection
   *
   */
  enum class __attribute__((packed)) boost : uint8_t {
    BOOST_3   = 3,  //!< ADC channel has current x2
    BOOST_2   = 2,  //!< ADC channel has current x1 (default)
    BOOST_066 = 1,  //!< ADC channel has current x2/3
    BOOST_05  = 0   //!< ADC channel has current x1/2
  };

  /**
   * @brief ADC Gain Selection
   *
   */
  enum class __attribute__((packed)) gain : uint8_t {
    GAIN_64  = 7,  //!< Gain is x64 (x16 analog, x4 digital)
    GAIN_32  = 6,  //!< Gain is x32 (x16 analog, x2 digital)
    GAIN_16  = 5,  //!< Gain is x16
    GAIN_8   = 4,  //!< Gain is x8
    GAIN_4   = 3,  //!< Gain is x4
    GAIN_2   = 2,  //!< Gain is x2
    GAIN_1   = 1,  //!< Gain is x1 (default)
    GAIN_033 = 0   //!< Gain is x 1/3
  };

  /**
   * @brief Conversion Mode Selection
   *
   */
  enum class __attribute__((packed)) conv_mode : uint8_t {
    CONTINUOUS = 3,  //!< Continuous Conversion mode or continuous conversion cycle in SCAN mode
    ONESHOT_STANDBY = 2,  //!< One-shot conversion or one-shot cycle in SCAN mode. It sets
                          //!< ADC_MODE[1:0] to ‘10’ (standby) at the end of the conversion or at
                          //!< the end of the conversion cycle in SCAN mode.
    ONESHOT_SHUTDOWN = 0  //!<  One-shot conversion or one-shot cycle in SCAN mode. It sets
                          //!<  ADC_MODE[1:0] to ‘0x’ (ADC Shutdown) at the end of the conversion or
                          //!<  at the end of the conversion cycle in SCAN mode (default).
  };

  /**
   * @brief ADC Output Data Format Selection
   *
   */
  enum class __attribute__((packed)) data_format : uint8_t {
    ID_SGNEXT_DATA =
        3,  //!< 32-bit (25-bit right justified data + Channel ID): CHID[3:0] + SGN extension (4
            //!< bits) + 24-bit ADC data. It allows overrange with the SGN extension.
    SGNEXT_DATA = 2,  //!< 32-bit (25-bit right justified data): SGN extension (8-bit) + 24-bit ADC
                      //!< data. It allows overrange with the SGN extension
    SGN_DATA_ZERO = 1,  //!< 32-bit (24-bit left justified data): 24-bit ADC data + 0x00 (8-bit). It
                        //!< does not allow overrange (ADC code locked to 0xFFFFFF or 0x800000).
    SGN_DATA = 0  //!< 24-bit (default ADC coding): 24-bit ADC data. It does not allow overrange
                  //!< (ADC code locked to 0xFFFFFF or 0x800000).
  };

  /**
   * @brief CRC Checksum Format Selection on Read Communications
   *
   */
  enum class __attribute__((packed)) crc_format : uint8_t {
    CRC_32 = 1,  //!< 32-bit wide (CRC-16 followed by 16 zeros)
    CRC_16 = 0   //!< 16-bit wide (CRC-16 only) (default)
  };

  /**
   * @brief MUX_VIN Input Selection
   *
   */
  enum class __attribute__((packed)) mux : uint8_t {
    MUX_VCM          = 15,  //!< Internal VCM
    MUX_TemperatureM = 14,  //!< Internal Temperature Sensor Diode M (Temp Diode M)
    MUX_TemperatureP = 13,  //!< Internal Temperature Sensor Diode P (Temp Diode P)
    MUX_REFINM       = 12,  //!< REFIN-
    MUX_REFINP       = 11,  //!< REFIN+
    MUX_AVDD         = 9,   //!< AVDD
    MUX_AGND         = 8,   //!< AGND
    MUX_CH7          = 7,   //!< CH7
    MUX_CH6          = 6,   //!< CH6
    MUX_CH5          = 5,   //!< CH5
    MUX_CH4          = 4,   //!< CH4
    MUX_CH3          = 3,   //!< CH3
    MUX_CH2          = 2,   //!< CH2
    MUX_CH1          = 1,   //!< CH1 (default vin-)
    MUX_CH0          = 0    //!< CH0 (default vin+)
  };

  /**
   * @brief Delay Time Between Each Conversion During a Scan Cycle
   *
   */
  enum class __attribute__((packed)) delay : uint8_t {
    DLY_512 = 7,  //!< 512 * DMCLK
    DLY_256 = 6,  //!< 256 * DMCLK
    DLY_128 = 5,  //!< 128 * DMCLK
    DLY_64  = 4,  //!< 64 * DMCLK
    DLY_32  = 3,  //!< 32 * DMCLK
    DLY_16  = 2,  //!< 16 * DMCLK
    DLY_8   = 1,  //!< 8 * DMCLK
    DLY_0   = 0   //!< 0: no delay (default)
  };

  /**
   * @brief configuration register 0
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1576710>MCP346x.pdf</a>
   */
  typedef union Config0 {
    struct {
      enum adc_mode adc : 2;  //!< ADC Operating Mode Selection
      enum cs_sel bias  : 2;  //!< Current Source/Sink Selection Bits for Sensor Bias
      enum clk_sel clk  : 2;  //!< Clock Selection
      bool vref_sel     : 1;  //!<
      uint8_t cfg0      : 1;  //!< Full Shutdown Mode Enable
    };
    uint8_t raw;  //!< raw access to register
  } config0_t;

  /**
   * @brief configuration register 1
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1269089>MCP346x.pdf</a>
   */
  typedef union config1 {
    struct {
      uint8_t      : 2;  //!< reserved
      enum osr osr : 4;  //!< Oversampling Ratio for Delta-Sigma A/D Conversion
      enum pre pre : 2;  //!< Prescaler Value Selection for AMCLK
    };
    uint8_t raw;  //!< raw access to register
  } config1_t;

  /**
   * @brief configuration register 2
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1269283>MCP346x.pdf</a>
   */
  typedef union Config2 {
    struct {
      uint8_t          : 2;  //!< reserved // Should always be equal to ‘11’
      bool az_mu       : 1;  //!< Auto-Zeroing MUX Setting
      enum gain gain   : 3;  //!< ADC Gain Selection
      enum boost boost : 2;  //!< ADC Bias Current Selection
    };
    uint8_t raw;  //!< raw access to register
  } config2_t;

  /**
   * @brief configuration register 3
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1269504>MCP346x.pdf</a>
   */
  typedef union Config3 {
    struct {
      bool en_gaincal              : 1;  //!< Enable Digital Gain Calibration
      bool en_offcal               : 1;  //!< Enable Digital Offset Calibration
      bool en_crccom               : 1;  //!< CRC Checksum Selection on Read Communications
      bool crc_format              : 1;  //!< CRC Checksum Format Selection on Read Communications
      enum data_format data_format : 2;  //!< ADC Output Data Format Selection
      enum conv_mode conv_mode     : 2;  //!< Conversion Mode Selection
    };
    uint8_t raw;  //!< raw access to register
  } config3_t;

  /**
   * @brief interrupt request register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1269747>MCP346x.pdf</a>
   */
  typedef union Irq {
    struct {
      bool en_stp        : 1;  //!< Enable Conversion Start Interrupt Output
      bool en_fastcmd    : 1;  //!< Enable Fast Commands in the COMMAND Byte
      uint8_t irq_mode   : 2;  //!< Configuration for the IRQ/MDAT Pin
      bool por_status    : 1;  //!< POR Status Flag
      bool crccfg_status : 1;  //!< CRC Error Status Flag Bit for Internal Registers
      bool dr_status     : 1;  //!< Data Ready Status Flag
      bool               : 1;  //!< unimplemented
    };
    uint8_t raw;  //!< raw access to register
  } irq_t;

  /**
   * @brief multiplexer register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1273028>MCP346x.pdf</a>
   */
  typedef union Mux {
    /**
     * @brief Construct a new Mux object
     *
     * @param data
     */
    struct {
      enum mux vin_minus : 4;  //!< MUX_VIN- Input Selection
      enum mux vin_plus  : 4;  //!< MUX_VIN+ Input Selection
    };
    uint8_t raw;  //!< raw access to register
  } mux_t;

  /**
   * @brief scan mode settings register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1270252>MCP346x.pdf</a>
   */
  typedef union Scan {
    struct {
      union {
        struct {
          uint8_t single_ended : 8;  //!< Single-Ended Channel CH0-CH7
          uint8_t differential : 4;  //!< Differential Channel A-D
          bool temp            : 1;  //!< TEMP
          bool avdd            : 1;  //!< AVDD
          bool vcm             : 1;  //!< VCM
          bool offset          : 1;  //!< OFFSET
        };
        uint16_t raw;  //!< raw access to register
      } channel;
      uint8_t        : 4;  //!< unimplemented: read as ‘0’
      bool           : 1;  //!< reserved: should be set to ‘0‘
      enum delay dly : 3;  //!< delay time between each conversion during a scan cycle
    };
    uint8_t raw[3];  //!< raw access to register
  } scan_t;

  /**
   * @brief timer delay value register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1270583>MCP346x.pdf</a>
   */
  typedef union Timer {
    uint8_t raw[3];  //!< Selection Bits for the Time Interval Between Two Consecutive Scan Cycles
  } timer_t;

  /**
   * @brief offset calibration register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1270742>MCP346x.pdf</a>
   */
  typedef union Offset {
    uint8_t raw[3];  //!< Offset Error Digital Calibration Code (two’s complement, MSb first coding)
  } offset_t;

  /**
   * @brief gain calibration register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1270900>MCP346x.pdf</a>
   */
  typedef union Gain {
    uint8_t raw[3];  //!< Gain Error Digital Calibration Code (unsigned, MSb first coding)
  } gain_t;

  /**
   * @brief SPI write mode locking password value register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1271641>MCP346x.pdf</a>
   */
  typedef union Lock {
    uint8_t raw;  //!< Write Access Password Entry Code
  } lock_t;

  /**
   * @brief crc configuration register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1272118>MCP346x.pdf</a>
   */
  typedef union Crccfg {
    uint8_t raw[2];  //!< CRC-16 Checksum Value
  } crccfg_t;

  struct {
    config0_t config0;      //!< register setting
    config1_t config1;      //!< register setting
    config2_t config2;      //!< register setting
    config3_t config3;      //!< register setting
    irq_t irq;              //!< register setting
    mux_t mux;              //!< register setting
    scan_t scan;            //!< register setting
    timer_t timer;          //!< register setting
    offset_t offsetcal;     //!< register setting
    gain_t gaincal;         //!< register setting
    uint8_t reserverd1[3];  //!< register setting
    uint8_t reserverd2;     //!< register setting
    lock_t lock;            //!< register setting
    uint16_t id;            //!< register setting
    crccfg_t crccfg;        //!< register setting
  };

  //  MCPSettings& operator=(const uint8_t* settings);

  MCPSettings(const config0_t CONFIG0, const config1_t CONFIG1, const config2_t CONFIG2,
              const config3_t CONFIG3, const irq_t IRQ, const mux_t MUX, const scan_t SCAN,
              const timer_t TIMER, const offset_t OFFSET, const gain_t GAIN, const lock_t LOCK,
              const crccfg_t CRCCFG);

  MCPSettings(const uint8_t CONFIG0 = _DEFAULT_CONFIG0, const uint8_t CONFIG1 = _DEFAULT_CONFIG1,
              const uint8_t CONFIG2 = _DEFAULT_CONFIG2, const uint8_t CONFIG3 = _DEFAULT_CONFIG3,
              const uint8_t IRQ = _DEFAULT_IRQ, const uint8_t MUX = _DEFAULT_MUX,
              const uint8_t* SCAN = _DEFAULT_SCAN, const uint8_t* TIMER = _DEFAULT_TIMER,
              const uint8_t* OFFSET = _DEFAULT_OFFSET, const uint8_t* GAIN = _DEFAULT_GAIN,
              const uint8_t LOCK = _DEFAULT_LOCK, const uint8_t* CRCCFG = _DEFAULT_CRCCFG);

  //  ~MCPSettings();

  void reset();
};

#endif
