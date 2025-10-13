// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Stefan Herold

/**
 * @file Arduino_MCP3x6x_Library.hpp
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief Library to support Microchip MPC3x6x/R 16/24bit analog to digital
 * converters.
 * @version 0.0.3
 * @date 2024-04-10
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SRC_ARDUINO_MCP3X6X_LIBRARY_HPP_
#define SRC_ARDUINO_MCP3X6X_LIBRARY_HPP_

#include <Arduino.h>
#include <SPI.h>

#ifdef MCP3x6x_DEBUG
#  ifndef MCP3x6x_DEBUG_INTERFACE
#    define MCP3x6x_DEBUG_INTERFACE Serial.println
#  endif
#  define MCP3x6x_LOG(msg) MCP3x6x_DEBUG_INTERFACE(msg)
#endif

#define MCP3x6x_OFFSET (byte)(0x88)  //!< corresponding mux setting
#define MCP3x6x_VCM    (byte)(0xF8)  //!< corresponding mux setting
#define MCP3x6x_AVDD   (byte)(0x98)  //!< corresponding mux setting
#define MCP3x6x_TEMP   (byte)(0xDE)  //!< corresponding mux setting
#define MCP3x6x_DIFFD  (byte)(0x67)  //!< corresponding mux setting
#define MCP3x6x_DIFFC  (byte)(0x45)  //!< corresponding mux setting
#define MCP3x6x_DIFFB  (byte)(0x23)  //!< corresponding mux setting
#define MCP3x6x_DIFFA  (byte)(0x01)  //!< corresponding mux setting
#define MCP3x6x_CH7    (byte)(0x78)  //!< corresponding mux setting
#define MCP3x6x_CH6    (byte)(0x68)  //!< corresponding mux setting
#define MCP3x6x_CH5    (byte)(0x58)  //!< corresponding mux setting
#define MCP3x6x_CH4    (byte)(0x48)  //!< corresponding mux setting
#define MCP3x6x_CH3    (byte)(0x38)  //!< corresponding mux setting
#define MCP3x6x_CH2    (byte)(0x28)  //!< corresponding mux setting
#define MCP3x6x_CH1    (byte)(0x18)  //!< corresponding mux setting
#define MCP3x6x_CH0    (byte)(0x08)  //!< corresponding mux setting

#define MCP3461_DEVICE_TYPE (uint16_t)(0x0008)  //!< MCP3461 device ID
#define MCP3462_DEVICE_TYPE (uint16_t)(0x0009)  //!< MCP3462 device ID
#define MCP3464_DEVICE_TYPE (uint16_t)(0x000B)  //!< MCP3464 device ID
#define MCP3561_DEVICE_TYPE (uint16_t)(0x000C)  //!< MCP3561 device ID
#define MCP3562_DEVICE_TYPE (uint16_t)(0x000D)  //!< MCP3562 device ID
#define MCP3564_DEVICE_TYPE (uint16_t)(0x000F)  //!< MCP3564 device ID

#ifndef MCP3x6x_SPI_ADR
#  define MCP3x6x_SPI_ADR (byte)(0x01000000)  //!< DEVICE ADDRESS
#endif

#define MCP3x6x_CMD_CONVERSION \
  (byte)(MCP3x6x_SPI_ADR | 0b101000)  //!< fast command
#define MCP3x6x_CMD_STANDBY \
  (byte)(MCP3x6x_SPI_ADR | 0b101100)  //!< fast command
#define MCP3x6x_CMD_SHUTDOWN \
  (byte)(MCP3x6x_SPI_ADR | 0b110000)  //!< fast command
#define MCP3x6x_CMD_FULL_SHUTDOWN \
  (byte)(MCP3x6x_SPI_ADR | 0b110100)                           //!< fast command
#define MCP3x6x_CMD_RESET  (byte)(MCP3x6x_SPI_ADR | 0b111000)  //!< fast command
#define MCP3x6x_CMD_SREAD  (byte)(MCP3x6x_SPI_ADR | 0b000001)  //!< fast command
#define MCP3x6x_CMD_IREAD  (byte)(MCP3x6x_SPI_ADR | 0b000011)  //!< fast command
#define MCP3x6x_CMD_IWRITE (byte)(MCP3x6x_SPI_ADR | 0b000010)  //!< fast command

#define MCP3x6x_ADR_ADCDATA \
  (byte)(MCP3x6x_SPI_ADR | (0x0 << 2))  //!< Register ADCDdata address
#define MCP3x6x_ADR_CONFIG0 \
  (byte)(MCP3x6x_SPI_ADR | (0x1 << 2))  //!< Register Config0 address
#define MCP3x6x_ADR_CONFIG1 \
  (byte)(MCP3x6x_SPI_ADR | (0x2 << 2))  //!< Register Config1 address
#define MCP3x6x_ADR_CONFIG2 \
  (byte)(MCP3x6x_SPI_ADR | (0x3 << 2))  //!< Register Config2 address
#define MCP3x6x_ADR_CONFIG3 \
  (byte)(MCP3x6x_SPI_ADR | (0x4 << 2))  //!< Register Config3 address
#define MCP3x6x_ADR_IRQ \
  (byte)(MCP3x6x_SPI_ADR | (0x5 << 2))  //!< Register IRQ address
#define MCP3x6x_ADR_MUX \
  (byte)(MCP3x6x_SPI_ADR | (0x6 << 2))  //!< Register MUX address
#define MCP3x6x_ADR_SCAN \
  (byte)(MCP3x6x_SPI_ADR | (0x7 << 2))  //!< Register SCAN address
#define MCP3x6x_ADR_TIMER \
  (byte)(MCP3x6x_SPI_ADR | (0x8 << 2))  //!< Register Timer address
#define MCP3x6x_ADR_OFFSET \
  (byte)(MCP3x6x_SPI_ADR | (0x9 << 2))  //!< Register OFFSET address
#define MCP3x6x_ADR_GAIN \
  (byte)(MCP3x6x_SPI_ADR | (0xA << 2))  //!< Register GAIN address
#define MCP3x6x_ADR_RESERVED1 \
  (byte)(MCP3x6x_SPI_ADR | (0xB << 2))  //!< reserved register
#define MCP3x6x_ADR_RESERVED2 \
  (byte)(MCP3x6x_SPI_ADR | (0xC << 2))  //!< reserved register
#define MCP3x6x_ADR_LOCK \
  (byte)(MCP3x6x_SPI_ADR | (0xD << 2))  //!< Register LOCK address
#define MCP3x6x_ADR_RESERVED3 \
  (byte)(MCP3x6x_SPI_ADR | (0xE << 2))  //!< reserved register
#define MCP3x6x_ADR_CRCCFG \
  (byte)(MCP3x6x_SPI_ADR | (0xF << 2))  //!< Register CRCCFG address

#ifndef MCP3x6x_CFG
#  define MCP3x6x_CFG
const uint8_t MCP3x6x_CFG_CONFIG0      = 0xC0;                //!< default value
const uint8_t MCP3x6x_CFG_CONFIG1      = 0x0C;                //!< default value
const uint8_t MCP3x6x_CFG_CONFIG2      = 0x8B;                //!< default value
const uint8_t MCP3x6x_CFG_CONFIG3      = 0x00;                //!< default value
const uint8_t MCP3x6x_CFG_IRQ          = 0x73;                //!< default value
const uint8_t MCP3x6x_CFG_MUX          = 0x01;                //!< default value
const uint8_t MCP3x6x_CFG_SCAN[3]      = {0x00, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_TIMER[3]     = {0x00, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_OFFSET[3]    = {0x00, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_GAIN[3]      = {0x80, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_RESERVED1[3] = {0x90, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_RESERVED2    = 0x50;                //!< default value
const uint8_t MCP3x6x_CFG_LOCK         = 0xA5;                //!< default value
const uint8_t MCP3x6x_CFG_CRCCFG[2]    = {0x00, 0x00};        //!< default value
#endif

namespace MCP {
/**
 * @brief ADC Operating Mode Selection
 *
 */
enum __attribute__((packed)) adc_mode {
  CONVERSION = 3,  //!< ADC Conversion mode
  STANDBY    = 2,  //!< ADC Standby mode
  SHUTDOWN   = 0   //!< ADC shutdown mode (default)
};

/**
 * @brief Current Source/Sink Selection Bits for Sensor Bias.
 *
 */
enum __attribute__((packed)) cs_sel {
  BIAS_15UA = 3,  //!< 15 μA is applied to the ADC inputs
  BIAS_37UA = 2,  //!< 3.7 μA is applied to the ADC inputs
  BIAS_09UA = 1,  //!< 0.9 μA is applied to the ADC inputs
  BIAS_0UA  = 0   //!< No current source is applied to the ADC inputs (default)
};

/**
 * @brief clock selection
 *
 */
enum __attribute__((packed)) clk_sel {
  INTERN_OUTPUT = 3,  //!< Internal clock is selected and AMCLK is present on
                      //!< the analog master clock output pin
  INTERN = 2,  //!< Internal clock is selected and no clock output is present on
               //!< the CLK pin
  EXTERN = 0   //!< External digital clock (default)
};

/**
 * @brief Oversampling Ratio for Delta-Sigma A/D Conversion
 *
 */
enum __attribute__((packed)) osr {
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
enum __attribute__((packed)) pre {
  MCLK_8 = 3,  //!< AMCLK = MCLK/8
  MCLK_4 = 2,  //!< AMCLK = MCLK/4
  MCLK_2 = 1,  //!< AMCLK = MCLK/2
  MCLK_0 = 0   //!< AMCLK = MCLK (default)
};

/**
 * @brief ADC Bias Current Selection
 *
 */
enum __attribute__((packed)) boost {
  BOOST_3   = 3,  //!< ADC channel has current x2
  BOOST_2   = 2,  //!< ADC channel has current x1 (default)
  BOOST_066 = 1,  //!< ADC channel has current x2/3
  BOOST_05  = 0   //!< ADC channel has current x1/2
};

/**
 * @brief ADC Gain Selection
 *
 */
enum __attribute__((packed)) gain {
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
enum __attribute__((packed)) conv_mode {
  CONTINUOUS = 3,  //!< Continuous Conversion mode or continuous conversion
                   //!< cycle in SCAN mode
  ONESHOT_STANDBY =
      2,  //!< One-shot conversion or one-shot cycle in SCAN mode. It sets
          //!< ADC_MODE[1:0] to ‘10’ (standby) at the end of the conversion or
          //!< at the end of the conversion cycle in SCAN mode.
  ONESHOT_SHUTDOWN =
      0  //!<  One-shot conversion or one-shot cycle in SCAN mode. It sets
         //!<  ADC_MODE[1:0] to ‘0x’ (ADC Shutdown) at the end of the conversion
         //!<  or at the end of the conversion cycle in SCAN mode (default).
};

/**
 * @brief ADC Output Data Format Selection
 *
 */
enum __attribute__((packed)) data_format {
  ID_SGNEXT_DATA = 3,  //!< 32-bit (25-bit right justified data + Channel ID):
                       //!< CHID[3:0] + SGN extension (4 bits) + 24-bit ADC
                       //!< data. It allows overrange with the SGN extension.
  SGNEXT_DATA =
      2,  //!< 32-bit (25-bit right justified data): SGN extension (8-bit) +
          //!< 24-bit ADC data. It allows overrange with the SGN extension
  SGN_DATA_ZERO = 1,  //!< 32-bit (24-bit left justified data): 24-bit ADC data
                      //!< + 0x00 (8-bit). It does not allow overrange (ADC code
                      //!< locked to 0xFFFFFF or 0x800000).
  SGN_DATA = 0  //!< 24-bit (default ADC coding): 24-bit ADC data. It does not
                //!< allow overrange (ADC code locked to 0xFFFFFF or 0x800000).
};

/**
 * @brief CRC Checksum Format Selection on Read Communications
 *
 */
enum __attribute__((packed)) crc_format {
  CRC_32 = 1,  //!< 32-bit wide (CRC-16 followed by 16 zeros)
  CRC_16 = 0   //!< 16-bit wide (CRC-16 only) (default)
};

/**
 * @brief MUX_VIN Input Selection
 *
 */
enum __attribute__((packed)) mux {
  MUX_VCM        = 15,  //!< Internal VCM
  MuxemperatureM = 14,  //!< Internal Temperature Sensor Diode M (Temp Diode M)
  MuxemperatureP = 13,  //!< Internal Temperature Sensor Diode P (Temp Diode P)
  MUX_REFINM     = 12,  //!< REFIN-
  MUX_REFINP     = 11,  //!< REFIN+
  MUX_AVDD       = 9,   //!< AVDD
  MUX_AGND       = 8,   //!< AGND
  MUX_CH7        = 7,   //!< CH7
  MUX_CH6        = 6,   //!< CH6
  MUX_CH5        = 5,   //!< CH5
  MUX_CH4        = 4,   //!< CH4
  MUX_CH3        = 3,   //!< CH3
  MUX_CH2        = 2,   //!< CH2
  MUX_CH1        = 1,   //!< CH1 (default vin-)
  MUX_CH0        = 0    //!< CH0 (default vin+)
};

/**
 * @brief Delay Time Between Each Conversion During a Scan Cycle
 *
 */
enum __attribute__((packed)) delay {
  DLY_512 = 7,  //!< 512 * DMCLK
  DLY_256 = 6,  //!< 256 * DMCLK
  DLY_128 = 5,  //!< 128 * DMCLK
  DLY_64  = 4,  //!< 64 * DMCLK
  DLY_32  = 3,  //!< 32 * DMCLK
  DLY_16  = 2,  //!< 16 * DMCLK
  DLY_8   = 1,  //!< 8 * DMCLK
  DLY_0   = 0   //!< 0: no delay (default)
};

}  // namespace MCP

/**
 * @brief base class MCP3x6x
 *
 */
class MCP3x6x {
 protected:
  const size_t _MAX_RESOLUTION = 0;
  const size_t _MAX_CHANNELS   = 0;

 private:
  typedef union __attribute__((__packed__)) {
    struct {
      struct {
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

  /**
   * @brief structure with latest value per channel
   *
   */
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
  } _result;  //!< todo

  status_t _fastcmd(uint8_t cmd) { return _transfer(0x00, cmd, 0); }
  void _reverse_array(uint8_t *array, size_t size);
  status_t _transfer(uint8_t *data, uint8_t addr, size_t size);
  //  status_t _transfer16(uint8_t *data, uint8_t addr);
  int32_t _getValue(int32_t raw);
  uint8_t _getChannel(uint32_t raw);

  SPIClass _spi;                                // SPI interface
  SPISettings _spiSettings;                     // SPI settings
  uint8_t _pinCS, _pinMISO, _pinMOSI, _pinCLK;  // SPI pins
  uint8_t _pinIRQ, _pinMCLK;

  float _reference = 2.4;
  size_t _resolution, _channel_count;
  uint16_t _channel_mask;

  const uint8_t _channelID[16] = {
      MCP3x6x_CH0,   MCP3x6x_CH1,   MCP3x6x_CH2,   MCP3x6x_CH3,
      MCP3x6x_CH4,   MCP3x6x_CH5,   MCP3x6x_CH6,   MCP3x6x_CH7,
      MCP3x6x_DIFFA, MCP3x6x_DIFFB, MCP3x6x_DIFFC, MCP3x6x_DIFFD,
      MCP3x6x_TEMP,  MCP3x6x_AVDD,  MCP3x6x_VCM,   MCP3x6x_OFFSET};

  ///////////////////////////////////////////////////////////////////////////////
  // registers
  ////////////////////////////////////////////////////////////////////////////////
  /**
   * @brief configuration register 0
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1576710>MCP346x.pdf</a>
   */
  union Config0 {
    struct {
      enum MCP::adc_mode adc : 2;  //!< ADC Operating Mode Selection
      enum MCP::cs_sel
          bias : 2;  //!< Current Source/Sink Selection Bits for Sensor Bias
      enum MCP::clk_sel clk : 2;  //!< Clock Selection
      bool vref_sel         : 1;  //!<
      uint8_t cfg0          : 1;  //!< Full Shutdown Mode Enable
    };
    uint8_t raw;  //!< raw access to register
    Config0(const uint8_t data) : raw(data) {}
  } _config0;

  /**
   * @brief configuration register 1
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1269089>MCP346x.pdf</a>
   */
  union Config1 {
    struct {
      uint8_t : 2;  //!< reserved
      enum MCP::osr
          osr : 4;  //!< Oversampling Ratio for Delta-Sigma A/D Conversion
      enum MCP::pre pre : 2;  //!< Prescaler Value Selection for AMCLK
    };
    uint8_t raw;  //!< raw access to register
    Config1(const uint8_t data) : raw(data) {}
  } _config1;

  /**
   * @brief configuration register 2
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1269283>MCP346x.pdf</a>
   */
  union Config2 {
    struct {
      uint8_t               : 2;  //!< reserved // Should always be equal to ‘1’
      bool az_ref           : 1;  //!< Auto-Zeroing Reference Buffer Setting
      bool az_mux           : 1;  //!< Auto-Zeroing MUX Setting
      enum MCP::gain gain   : 3;  //!< ADC Gain Selection
      enum MCP::boost boost : 2;  //!< ADC Bias Current Selection
    };
    uint8_t raw;  //!< raw access to register
    Config2(const uint8_t data) : raw(data) {}
  } _config2;

  /**
   * @brief configuration register 3
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1269504>MCP346x.pdf</a>
   */
  union Config3 {
    struct {
      bool en_gaincal : 1;  //!< Enable Digital Gain Calibration
      bool en_offcal  : 1;  //!< Enable Digital Offset Calibration
      bool en_crccom  : 1;  //!< CRC Checksum Selection on Read Communications
      bool crc_format : 1;  //!< CRC Checksum Format Selection on Read
                            //!< Communications
      enum MCP::data_format
          data_format               : 2;  //!< ADC Output Data Format Selection
      enum MCP::conv_mode conv_mode : 2;  //!< Conversion Mode Selection
    };
    uint8_t raw;  //!< raw access to register
    Config3(const uint8_t data) : raw(data) {}
  } _config3;

  /**
   * @brief interrupt request register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1269747>MCP346x.pdf</a>
   */
  union Irq {
    struct {
      bool en_stp        : 1;  //!< Enable Conversion Start Interrupt Output
      bool en_fastcmd    : 1;  //!< Enable Fast Commands in the COMMAND Byte
      uint8_t irq_mode   : 2;  //!< Configuration for the IRQ/MDAT Pin
      bool por_status    : 1;  //!< POR Status Flag
      bool crccfg_status : 1;  //!< CRC Error Status Flag Bit for Internal
                               //!< Registers
      bool dr_status     : 1;  //!< Data Ready Status Flag
      bool               : 1;  //!< unimplemented
    };
    uint8_t raw;  //!< raw access to register
    Irq(const uint8_t data) : raw(data) {}
  } _irq;

  /**
   * @brief multiplexer register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1273028>MCP346x.pdf</a>
   */
  union Mux {
    struct {
      enum MCP::mux vin_minus : 4;  //!< MUX_VIN- Input Selection
      enum MCP::mux vin_plus  : 4;  //!< MUX_VIN+ Input Selection
    };
    uint8_t raw;  //!< raw access to register
    Mux(const uint8_t data = MCP3x6x_CFG_MUX) : raw(data) {}
  } _mux;

  /**
   * @brief scan mode settings register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1270252>MCP346x.pdf</a>
   */
  union Scan {
    struct __attribute__((packed)) {
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
      uint8_t unimplemented : 4;  //!< unimplemented: read as ‘0’
      bool reserved         : 1;  //!< reserved: should be set to ‘0‘
      enum MCP::delay
          dly : 3;  //!< delay time between each conversion during a scan cycle
    };
    uint8_t raw[3];  //!< raw access to register
    Scan(const uint8_t data[3] = MCP3x6x_CFG_SCAN)
        : raw{data[0], data[1], data[2]} {}
  } _scan;

  /**
   * @brief timer delay value register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1270583>MCP346x.pdf</a>
   */
  union Timer {
    uint8_t raw[3];  //!< Selection Bits for the Time Interval Between Two
                     //!< Consecutive Scan Cycles
    Timer(const uint8_t data[3] = MCP3x6x_CFG_TIMER)
        : raw{data[0], data[1], data[2]} {}
  } _timer;

  /**
   * @brief offset calibration register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1270742>MCP346x.pdf</a>
   */
  union Offset {
    uint8_t raw[3];  //!< Offset Error Digital Calibration Code (two’s
                     //!< complement, MSb first coding)
    Offset(const uint8_t data[3] = MCP3x6x_CFG_OFFSET)
        : raw{data[0], data[1], data[2]} {}
  } _offset;

  /**
   * @brief gain calibration register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1270900>MCP346x.pdf</a>
   */
  union Gain {
    uint8_t raw[3];  //!< Gain Error Digital Calibration Code (unsigned, MSb
                     //!< first coding)
    Gain(const uint8_t data[3] = MCP3x6x_CFG_GAIN)
        : raw{data[0], data[1], data[2]} {}
  } _gain;

  /**
   * @brief SPI write mode locking password value register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1271641>MCP346x.pdf</a>
   */
  union Lock {
    uint8_t raw;  //!< Write Access Password Entry Code
    Lock(const uint8_t data) : raw(data) {}
  } _lock;

  /**
   * @brief crc configuration register
   *
   * <a
   * href=https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP3461-2-4R-Family-Data-Sheet-DS20006404C.pdf#G1.1272118>MCP346x.pdf</a>
   */
  union Crccfg {
    union {
      uint16_t value;
      uint8_t raw[2];  //!< CRC-16 Checksum Value
    };
    Crccfg(const uint8_t data[2]) : raw{data[0], data[1]} {}
  } _crccfg;

  /**
   * @brief latest ADC value and channel
   *
   */
  struct Adcdata {
    uint8_t channelid : 4;   //!< channel ID
    int32_t value     : 25;  //!< actual value of conversion
  } _adcdata = {.channelid = MCP3x6x_CH0, .value = 0};  //!< todo

 public:
  ///////////////////////////////////////////////////////////////////////////////
  // de-constructor
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Construct a new MCP3x6x object
   *
   * @param pinCS
   * @param theSPI
   * @param theSPISettings
   * @param pinMOSI
   * @param pinMISO
   * @param pinCLK
   */
  MCP3x6x(const size_t resolution, const size_t channels, uint8_t pinCS,
          uint8_t pinMISO, uint8_t pinMOSI, uint8_t pinCLK, SPIClass theSPI,
          SPISettings theSPISettings);

  /**
   * @brief Destroy the MCP3x6x object
   *
   */
  ~MCP3x6x() {
    detachInterrupt(digitalPinToInterrupt(_pinIRQ));
    noTone(_pinMCLK);
    end();
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  ////////////////////////////////////////////////////////////////////////////////

  bool begin();

  /**
   * @brief end communication
   *
   */
  void end() { _spi.end(); }

  void configure(Config0 config0, Config1 config1, Config2 config2,
                 Config3 config3, Irq irq, Mux mux, Scan scan, Timer timer,
                 Offset offset, Gain gain, Lock lock, Crccfg crccfg);

  ///////////////////////////////////////////////////////////////////////////////
  // attach pins
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief attaches pin to interrupt callback function
   *
   * @param _pinIRQ
   * @param callback
   */
  void attachIRQ(const uint8_t _pinIRQ, void (*callback)(void));

  /**
   * @brief attached pin to clock generator
   *
   * @param _pinMCLK
   */
  void attachMCLK(const uint8_t _pinMCLK);

  ///////////////////////////////////////////////////////////////////////////////
  // set configuration registers
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief write register CONFIG0 to device
   *
   * @param adc
   * @param bias
   * @param clk
   * @param vref_sel
   */
  void config0(enum MCP::adc_mode adc, enum MCP::cs_sel bias,
               enum MCP::clk_sel clk, bool vref_sel);

  /**
   * @brief write register CONFIG1 to device
   *
   * @param osr
   * @param pre
   */
  void config1(enum MCP::osr osr, enum MCP::pre pre);

  /**
   * @brief write register CONFIG2 to device
   *
   * @param az_mux
   * @param gain
   * @param boost
   */
  void config2(bool az_mux, enum MCP::gain gain, enum MCP::boost boost);

  /**
   * @brief write register CONFIG3 to device
   *
   * @param gaincal
   * @param offcal
   * @param crccom
   * @param data_format
   * @param conv_mode
   */
  void config3(bool gaincal, bool offcal, bool crccom,
               enum MCP::data_format data_format,
               enum MCP::conv_mode conv_mode);

  /**
   * @brief write register IRQ to device
   *
   * @param stp
   * @param fastcmd
   * @param por_status
   * @param irq_mode
   */
  void irq(bool stp, bool fastcmd, uint8_t irq_mode);

  /**
   * @brief write register MUX to device
   *
   * @param minus
   * @param plus
   */
  void mux(enum MCP::mux minus, enum MCP::mux plus);

  /**
   * @brief write register SCAN to device
   *
   * @param single_ended
   * @param differential
   * @param temp
   * @param avdd
   * @param vcm
   * @param offset
   * @param dly
   */
  void scan(byte single_ended, byte differential, bool temp, bool avdd,
            bool vcm, bool offset, enum MCP::delay dly);

  /**
   * @brief write register TIMER to device
   *
   * @param timer
   */
  void timer(uint8_t *timer);

  /**
   * @brief write register OFFSET to device
   *
   * @param offset
   */
  void offset(uint8_t *offset);

  /**
   * @brief write register GAIN to device
   *
   * @param gain
   */
  void gain(uint8_t *gain);

  /**
   * @brief write register LOCK to device
   *
   * @param lock
   */
  void lock(uint8_t lock);

  /**
   * @brief write register LOCK to unlock device
   *
   * @param key
   */
  void unlock();

  /**
   * @brief write register CRCCFG to device
   *
   * @param crccfg
   */
  void crccfg(uint16_t crccfg);

  ///////////////////////////////////////////////////////////////////////////////
  // read status
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief data ready status of latest communication
   *
   * @return true
   * @return false
   */
  inline constexpr bool status_dr() { return !_status.dr; }

  /**
   * @brief crccfg status of latest communication
   *
   * @return true
   * @return false
   */
  inline constexpr bool status_crccfg() { return !_status.crccfg; }

  /**
   * @brief power on reset status of latest communication
   *
   * @return true
   * @return false
   */
  inline constexpr bool status_por() { return !_status.por; }

  ///////////////////////////////////////////////////////////////////////////////
  // fast commands
  ////////////////////////////////////////////////////////////////////////////////

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
  inline status_t full_shutdown() {
    return _fastcmd(MCP3x6x_CMD_FULL_SHUTDOWN);
  }

  /**
   * @brief Fast Command
   *
   * @return status_t
   */
  inline status_t reset() {
    //    configure();
    return _fastcmd(MCP3x6x_CMD_RESET);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // write registers
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief write config0 to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Config0 data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, 1);
  }

  /**
   * @brief write config1 to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Config1 data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1, 1);
  }

  /**
   * @brief write config2 to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Config2 data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, 1);
  }

  /**
   * @brief write config3 to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Config3 data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, 1);
  }

  /**
   * @brief write IRQ to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Irq data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, 1);
  }

  /**
   * @brief write MUX to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Mux data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, 1);
  }

  /**
   * @brief write SCAN to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Scan data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, 3);
  }

  /**
   * @brief write TIMER to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Timer data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_TIMER, 3);
  }

  /**
   * @brief write OFFSET to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Offset data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_OFFSET, 3);
  }

  /**
   * @brief write GAIN to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Gain data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_GAIN, 3);
  }

  /**
   * @brief write LOCK to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Lock data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, 1);
  }

  /**
   * @brief write CRCCFG to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Crccfg data) {
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CRCCFG, 2);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // read registers
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief read register ADCDATA from ADC
   *
   * @param data
   * @return status_t
   */
  status_t read(Adcdata *data);

  /**
   * @brief read register CONFIG0 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Config0 data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, 1);
  }

  /**
   * @brief read register CONFIG1 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Config1 data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1, 1);
  }

  /**
   * @brief read register CONFIG2 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Config2 data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2, 1);
  }

  /**
   * @brief read register CONFIG3 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Config3 data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3, 1);
  }

  /**
   * @brief read register IRQ from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Irq data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ, 1);
  }

  /**
   * @brief read register MUX from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Mux data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX, 1);
  }

  /**
   * @brief read register SCAN from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Scan data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_SCAN, 3);
  }

  /**
   * @brief read register TIMER from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Timer data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_TIMER, 3);
  }

  /**
   * @brief read register OFFSET from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Offset data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_OFFSET, 3);
  }

  /**
   * @brief read register GAIN from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Gain data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_GAIN, 3);
  }

  /**
   * @brief read register LOCK from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Lock data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK, 1);
  }

  /**
   * @brief read register CRCCFG from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Crccfg data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CRCCFG, 2);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief handler
   *
   */
  void IRQ_handler();

  /**
   * @brief Set the Data Format
   *
   * @param format
   */
  void setDataFormat(MCP::data_format format);

  /**
   * @brief Set the Conversion Mode
   *
   * @param mode
   */
  void setConversionMode(MCP::conv_mode mode);

  /**
   * @brief Set the Adc Mode object
   *
   * @param mode
   */
  void setAdcMode(MCP::adc_mode mode);

  /**
   * @brief Set the Clock Selection object
   *
   * @param clk
   */
  void setClockSelection(MCP::clk_sel clk);

  /**
   * @brief enable scanning on given channel
   *
   * @param ch
   */
  void enableScanChannel(Mux chan);

  /**
   * @brief disable scanning of given channel
   *
   * @param ch
   */
  void disableScanChannel(Mux chan);

  /**
   * @brief set oversampling rate
   *
   * @param rate
   */
  void setAveraging(MCP::osr rate);

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
  int32_t analogRead(Mux chan);

  /**
   * @brief read
   *
   * @param ch
   * @return int32_t
   */
  int32_t analogReadContinuous(Mux chan);

  /**
   * @brief mux
   *
   * @param pinP
   * @param pinN
   * @return int32_t
   */
  int32_t analogReadDifferential(Mux pinP, Mux pinN);
};

///////////////////////////////////////////////////////////////////////////////
// concrete classes
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief inherited class
 *
 */
class MCP3461 : public MCP3x6x {
 public:
  /**
   * @brief Construct a new MCP3461 object
   *
   * @param pinCS
   * @param pinMISO
   * @param pinMOSI
   * @param pinCLK
   * @param theSPI
   * @param theSPISettings
   */
  MCP3461(uint8_t pinCS = SS, uint8_t pinMISO = MISO, uint8_t pinMOSI = MOSI,
          uint8_t pinCLK = SCK, SPIClass theSPI = SPI,
          SPISettings theSPISettings = SPISettings())
      : MCP3x6x(16, 2, pinCS, pinMISO, pinMOSI, pinCLK, theSPI,
                theSPISettings) {}
};

/**
 * @brief inherited class
 *
 */
class MCP3462 : public MCP3x6x {
 public:
  /**
   * @brief Construct a new MCP3462 object
   *
   * @param pinCS
   * @param pinMISO
   * @param pinMOSI
   * @param pinCLK
   * @param theSPI
   * @param theSPISettings
   */
  MCP3462(uint8_t pinCS = SS, uint8_t pinMISO = MISO, uint8_t pinMOSI = MOSI,
          uint8_t pinCLK = SCK, SPIClass theSPI = SPI,
          SPISettings theSPISettings = SPISettings())
      : MCP3x6x(16, 4, pinCS, pinMISO, pinMOSI, pinCLK, theSPI,
                theSPISettings) {}
};

/**
 * @brief inherited class
 *
 */
class MCP3464 : public MCP3x6x {
 public:
  /**
   * @brief Construct a new MCP3464 object
   *
   * @param pinCS
   * @param pinMISO
   * @param pinMOSI
   * @param pinCLK
   * @param theSPI
   * @param theSPISettings
   */
  MCP3464(uint8_t pinCS = SS, uint8_t pinMISO = MISO, uint8_t pinMOSI = MOSI,
          uint8_t pinCLK = SCK, SPIClass theSPI = SPI,
          SPISettings theSPISettings = SPISettings())
      : MCP3x6x(16, 8, pinCS, pinMISO, pinMOSI, pinCLK, theSPI,
                theSPISettings) {}
};

/**
 * @brief inherited class
 *
 */
class MCP3561 : public MCP3x6x {
 public:
  /**
   * @brief Construct a new MCP3561 object
   *
   * @param pinCS
   * @param pinMISO
   * @param pinMOSI
   * @param pinCLK
   * @param theSPI
   * @param theSPISettings
   */
  MCP3561(uint8_t pinCS = SS, uint8_t pinMISO = MISO, uint8_t pinMOSI = MOSI,
          uint8_t pinCLK = SCK, SPIClass theSPI = SPI,
          SPISettings theSPISettings = SPISettings())
      : MCP3x6x(24, 2, pinCS, pinMISO, pinMOSI, pinCLK, theSPI,
                theSPISettings) {}
};

/**
 * @brief inherited class
 *
 */
class MCP3562 : public MCP3x6x {
 public:
  /**
   * @brief Construct a new MCP3562 object
   *
   * @param pinCS
   * @param pinMISO
   * @param pinMOSI
   * @param pinCLK
   * @param theSPI
   * @param theSPISettings
   */
  MCP3562(uint8_t pinCS = SS, uint8_t pinMISO = MISO, uint8_t pinMOSI = MOSI,
          uint8_t pinCLK = SCK, SPIClass theSPI = SPI,
          SPISettings theSPISettings = SPISettings())
      : MCP3x6x(24, 4, pinCS, pinMISO, pinMOSI, pinCLK, theSPI,
                theSPISettings) {}
};

/**
 * @brief inherited class
 *
 */
class MCP3564 : public MCP3x6x {
 public:
  /**
   * @brief Construct a new MCP3564 object
   *
   * @param pinCS
   * @param pinMISO
   * @param pinMOSI
   * @param pinCLK
   * @param theSPI
   * @param theSPISettings
   */
  MCP3564(uint8_t pinCS = SS, uint8_t pinMISO = MISO, uint8_t pinMOSI = MOSI,
          uint8_t pinCLK = SCK, SPIClass theSPI = SPI,
          SPISettings theSPISettings = SPISettings())
      : MCP3x6x(24, 8, pinCS, pinMISO, pinMOSI, pinCLK, theSPI,
                theSPISettings) {}
};

#endif  // SRC_ARDUINO_MCP3X6X_LIBRARY_HPP_
