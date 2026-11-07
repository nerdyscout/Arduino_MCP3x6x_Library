// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

/**
 * @file Arduino_MCP3x6x_Library.hpp
 * @author nerdyscout (stefan.herold@posteo.de)
 * @brief Library to support Microchip MCP3x6x/R 16/24bit ADCs
 * @version 0.2.0
 * @date 2026-03-23
 *
 * @copyright Copyright (c) 2026
 *
 * @mainpage Arduino MCP3x6x Library
 *
 * @section intro_sec Introduction
 * Arduino library for Microchip MCP346xR / MCP356xR 16/24-bit SAR
 * analog-to-digital converters over SPI.
 *
 * Supported devices:
 * - MCP3461R / MCP3462R / MCP3464R (16-bit)
 * - MCP3561R / MCP3562R / MCP3564R (24-bit)
 *
 * @section install_sec Installation
 * Use the Arduino Library Manager and search for **MCP3x6xR**, or clone this
 * repository into your `libraries` folder:
 *
 * @code{.sh}
 *   git clone https://github.com/nerdyscout/Arduino_MCP3x6x_Library.git \
 *     ~/Arduino/libraries/MCP3x6xR
 * @endcode
 *
 * @section example_sec Quick Example
 *
 * @code{.cpp}
 *   #include "Arduino_MCP3x6x_Library.hpp"
 *   MCP3461 mcp;
 *   mcp.begin();
 *   int32_t raw  = mcp.analogRead(MCP3x6x_CH0);
 *   double volt  = raw * mcp.getReference() / mcp.getMaxValue();
 * @endcode
 *
 * @section api_sec API Reference
 * - @ref init "Initialization" — begin(), end(), reset()
 * - @ref config "Configuration" — config0() … config3(), lock(), unlock()
 * - @ref pins "Pin assignment" — attachIRQ(), attachMCLK()
 * - @ref conversion "Conversion & reading" — analogRead(), startScan(), startContinuous()
 * - @ref status "Status & information" — status_dr(), dataAvailable(), toVoltage()
 * - @ref calibration "Calibration" — offset(), gain(), setTempOffset(), setTempSlope()
 * - @ref lowpower "Low-power modes" — standby(), shutdown(), full_shutdown()
 *
 * Full API reference is generated with Doxygen and hosted at:
 * https://nerdyscout.github.io/Arduino_MCP3x6x_Library/
 */

#ifndef SRC_ARDUINO_MCP3X6X_LIBRARY_HPP_
#define SRC_ARDUINO_MCP3X6X_LIBRARY_HPP_

#ifdef PIO_NATIVE_TESTING
  #include <ArduinoFake.h>
#else
  #include <Arduino.h>
  #include <SPI.h>
  #ifdef ARDUINO_ARCH_SAMD
    #include <wiring_private.h>
  #endif
  #ifdef ARDUINO_ARCH_ESP32
    #include "driver/spi_master.h"
  #endif
#endif

#ifdef MCP3x6x_DEBUG
  #ifndef MCP3x6x_DEBUG_INTERFACE
    #define MCP3x6x_DEBUG_INTERFACE Serial.print
  #endif
  #define MCP3x6x_LOG(...) MCP3x6x_DEBUG_INTERFACE(__VA_ARGS__)
#else
  #define MCP3x6x_LOG(...)
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

// (device ID defines removed — unused; device type is a template parameter)

#ifndef MCP3x6x_SPI_ADR
  #define MCP3x6x_SPI_ADR (byte)(0b01000000)  //!< DEVICE ADDRESS
#endif

#define MCP3x6x_CMD_CONVERSION    (byte)(MCP3x6x_SPI_ADR | 0b101000)  //!< fast command
#define MCP3x6x_CMD_STANDBY       (byte)(MCP3x6x_SPI_ADR | 0b101100)  //!< fast command
#define MCP3x6x_CMD_SHUTDOWN      (byte)(MCP3x6x_SPI_ADR | 0b110000)  //!< fast command
#define MCP3x6x_CMD_FULL_SHUTDOWN (byte)(MCP3x6x_SPI_ADR | 0b110100)  //!< fast command
#define MCP3x6x_CMD_RESET         (byte)(MCP3x6x_SPI_ADR | 0b111000)  //!< fast command
#define MCP3x6x_CMD_SREAD         (byte)(MCP3x6x_SPI_ADR | 0b000001)  //!< fast command
#define MCP3x6x_CMD_IREAD         (byte)(MCP3x6x_SPI_ADR | 0b000011)  //!< fast command
#define MCP3x6x_CMD_IWRITE        (byte)(MCP3x6x_SPI_ADR | 0b000010)  //!< fast command

#define MCP3x6x_ADR_ADCDATA   (byte)(MCP3x6x_SPI_ADR | (0x0 << 2))  //!< Register ADCDATA address
#define MCP3x6x_ADR_CONFIG0   (byte)(MCP3x6x_SPI_ADR | (0x1 << 2))  //!< Register Config0 address
#define MCP3x6x_ADR_CONFIG1   (byte)(MCP3x6x_SPI_ADR | (0x2 << 2))  //!< Register Config1 address
#define MCP3x6x_ADR_CONFIG2   (byte)(MCP3x6x_SPI_ADR | (0x3 << 2))  //!< Register Config2 address
#define MCP3x6x_ADR_CONFIG3   (byte)(MCP3x6x_SPI_ADR | (0x4 << 2))  //!< Register Config3 address
#define MCP3x6x_ADR_IRQ       (byte)(MCP3x6x_SPI_ADR | (0x5 << 2))  //!< Register IRQ address
#define MCP3x6x_ADR_MUX       (byte)(MCP3x6x_SPI_ADR | (0x6 << 2))  //!< Register MUX address
#define MCP3x6x_ADR_SCAN      (byte)(MCP3x6x_SPI_ADR | (0x7 << 2))  //!< Register SCAN address
#define MCP3x6x_ADR_TIMER     (byte)(MCP3x6x_SPI_ADR | (0x8 << 2))  //!< Register Timer address
#define MCP3x6x_ADR_OFFSET    (byte)(MCP3x6x_SPI_ADR | (0x9 << 2))  //!< Register OFFSET address
#define MCP3x6x_ADR_GAIN      (byte)(MCP3x6x_SPI_ADR | (0xA << 2))  //!< Register GAIN address
#define MCP3x6x_ADR_RESERVED1 (byte)(MCP3x6x_SPI_ADR | (0xB << 2))  //!< reserved register
#define MCP3x6x_ADR_RESERVED2 (byte)(MCP3x6x_SPI_ADR | (0xC << 2))  //!< reserved register
#define MCP3x6x_ADR_LOCK      (byte)(MCP3x6x_SPI_ADR | (0xD << 2))  //!< Register LOCK address
#define MCP3x6x_ADR_RESERVED3 (byte)(MCP3x6x_SPI_ADR | (0xE << 2))  //!< reserved register
#define MCP3x6x_ADR_CRCCFG    (byte)(MCP3x6x_SPI_ADR | (0xF << 2))  //!< Register CRCCFG address

#define MCP3x6x_TIMEOUT INT32_MIN

#ifndef MCP3x6x_CFG
  #define MCP3x6x_CFG
const uint8_t MCP3x6x_CFG_CONFIG0      = 0xC0;                //!< default value
const uint8_t MCP3x6x_CFG_CONFIG1      = 0x0C;                //!< default value
const uint8_t MCP3x6x_CFG_CONFIG2      = 0x8B;                //!< default value
const uint8_t MCP3x6x_CFG_CONFIG3      = 0x00;                //!< default value
const uint8_t MCP3x6x_CFG_IRQ          = 0x73;                //!< default value
const uint8_t MCP3x6x_CFG_MUX          = 0x08;                //!< default value (CH0+/AGND-)
const uint8_t MCP3x6x_CFG_SCAN[3]      = {0x00, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_TIMER[3]     = {0x00, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_OFFSET[3]    = {0x00, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_GAIN[3]      = {0x80, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_RESERVED1[3] = {0x90, 0x00, 0x00};  //!< default value
const uint8_t MCP3x6x_CFG_RESERVED2    = 0x50;                //!< default value
const uint8_t MCP3x6x_CFG_LOCK =
    0xA5;  //!< default value at POR (unlocked; 0xA5 = write access allowed)
const uint8_t MCP3x6x_CFG_CRCCFG[2] = {0x00, 0x00};  //!< default value
#endif

/**
 * @brief single sample captured by the DMA engine
 *
 */
struct DmaSample {
  uint8_t channel;     //!< channel ID decoded from ADCDATA[31:28] (ID_SGNEXT_DATA)
  int32_t value;       //!< sign-extended ADC value
  uint32_t timestamp;  //!< micros()/millis() when the sample was captured
};

/**
 * @brief lock-free single-producer/single-consumer ring buffer for DMA samples
 *
 * The DMA completioninterrupt produces (push), the user context consumes
 * (pop). Storage is caller-provided; no heap is used. Pushing into a full
 * buffer drops the sample (returns false) — it never blocks.
 */
class DmaRingBuffer {
 public:
  void setup(DmaSample* buf, size_t cap) {
    _buf = buf;
    _cap = cap;
    clear();
  }
  bool push(DmaSample s) {
    if (_buf == nullptr || _count >= _cap) {
      return false;
    }
    _buf[(_head + _count) % _cap] = s;
    _count++;
    return true;
  }
  bool pop(DmaSample& s) {
    if (_buf == nullptr || _count == 0) {
      return false;
    }
    s     = _buf[_head];
    _head = (_head + 1) % _cap;
    _count--;
    return true;
  }
  size_t available() const { return _count; }
  void clear() {
    _head  = 0;
    _count = 0;
  }

 private:
  DmaSample* _buf        = nullptr;
  size_t _cap            = 0;
  volatile size_t _head  = 0;
  volatile size_t _count = 0;
};

/**
 * @defgroup init Initialization
 * @brief begin(), end(), reset() — initialize and manage the ADC.
 *
 * @defgroup config Configuration registers
 * @brief config0() … config3(), irq(), mux(), scan(), timer(), lock(), unlock(), crccfg() —
 *        program the MCP3x6x/R configuration registers.
 *
 * @defgroup pins Pin assignment
 * @brief attachIRQ(), attachMCLK() — connect IRQ and MCLK pins.
 *
 * @defgroup conversion Conversion & reading
 * @brief analogRead(), analogReadContinuous(), analogReadDifferential(),
 *        readADCDATA(), startContinuous(), stopContinuous(), startScan(),
 *        startScanOnce(), singleEndedMode(), differentialMode() — trigger and
 *        retrieve conversions.
 *
 * @defgroup status Status & information
 * @brief status_dr(), status_crccfg(), status_por(), dataAvailable(),
 *        isComplete(), isContinuous(), isDifferential(), toVoltage(),
 *        getReference(), getMaxValue(), getReadTimeout(), setReadTimeout()
 *        — query conversion status and device parameters.
 *
 * @defgroup calibration Calibration
 * @brief offset(), gain(), setTempOffset(), setTempSlope(), setAveraging(),
 *        setOversamplingRatio() — calibration and offset/gain adjustment.
 *
 * @defgroup lowpower Low-power modes
 * @brief standby(), shutdown(), full_shutdown() — reduce power consumption.
 *
 * @defgroup dma DMA support
 * @brief attachDMA(), dmaAvailable(), dmaRead() — direct memory access for
 *        high-speed continuous conversion (ESP32 only).
 *
 * @defgroup version Version info
 * @brief getDeviceAddress(), available(), read(), write(), peek(), flush()
 *        — low-level SPI access and device identification.
 */

/**
 * @brief base class MCP3x6x
 *
 * Template base class for Microchip MCP346xR / MCP356xR ADCs.
 *
 * @tparam _MAX_RESOLUTION maximum ADC resolution in bits (16 or 24)
 * @tparam _MAX_CHANNELS   maximum number of input channels (1, 2, or 4)
 *
 * @ingroup init config pins conversion status calibration lowpower dma version
 */
template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
class MCP3x6x {
#ifdef PIO_NATIVE_TESTING
  friend struct MCP3x6xTest;
#endif
 public:
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
    INTERN_OUTPUT = 3,  //!< Internal clock is selected and AMCLK is present on the analog master
                        //!< clock output pin
    INTERN = 2,  //!< Internal clock is selected and no clock output is present on the CLK pin
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
   * @brief IRQ/MDAT Pin Configuration
   *
   */
  enum irq_mode : uint8_t {
    IRQ_OFF            = 0,  //!< Hi-Z (pin disabled)
    IRQ_OPEN_DRAIN     = 1,  //!< IRQ output, open drain, active low
    IRQ_PUSH_PULL_LOW  = 2,  //!< IRQ output, push-pull, active low
    IRQ_PUSH_PULL_HIGH = 3   //!< IRQ output, push-pull, active high
  };

  /**
   * @brief Conversion Mode Selection
   *
   */
  enum __attribute__((packed)) conv_mode {
    CONTINUOUS       = 3,  //!< Continuous Conversion mode or continuous conversion
                           //!< cycle in SCAN mode
    ONESHOT_STANDBY  = 2,  //!< One-shot conversion or one-shot cycle in SCAN mode. It sets
                           //!< ADC_MODE[1:0] to ‘10’ (standby) at the end of the conversion or
                           //!< at the end of the conversion cycle in SCAN mode.
    ONESHOT_SHUTDOWN = 0   //!<  One-shot conversion or one-shot cycle in SCAN mode. It sets
                           //!<  ADC_MODE[1:0] to ‘0x’ (ADC Shutdown) at the end of the
                           //!<  conversion or at the end of the conversion cycle in SCAN mode
                           //!<  (default).
  };

  /**
   * @brief ADC Output Data Format Selection
   *
   */
  enum __attribute__((packed)) data_format {
    ID_SGNEXT_DATA = 3,  //!< 32-bit (25-bit right justified data + Channel ID):
                         //!< CHID[3:0] + SGN extension (4 bits) + 24-bit ADC data.
                         //!< It allows overrange with the SGN extension.
    SGNEXT_DATA    = 2,  //!< 32-bit (25-bit right justified data): SGN extension (8-bit) +
                         //!< 24-bit ADC data. It allows overrange with the SGN extension
    SGN_DATA_ZERO =
        1,        //!< 32-bit (24-bit left justified data): 24-bit ADC data 0x00 (8-bit).
                  //!< It does not allow overrange (ADC code < locked to 0xFFFFFF or 0x800000).
    SGN_DATA = 0  //!< 24-bit (default ADC coding): 24-bit ADC data.
                  //!< It does not allow overrange (ADC code locked to 0xFFFFFF or 0x800000).
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

 private:
  typedef union __attribute__((__packed__)) {
    struct {
      bool por     : 1;  //!< status: power on reset (bit 0)
      bool crccfg  : 1;  //!< status: crc error (bit 1)
      bool dr      : 1;  //!< status: data ready (bit 2)
      uint8_t addr : 3;  //!< inverted device address ack (bits 5:3)
      uint8_t      : 2;  //!< reserved, always 0 (bits 7:6)
    };
    uint8_t raw = 0;
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

  status_t _fastcmd(uint8_t cmd) {
    byte data = 0x00;
    return _transfer(&data, cmd, 0);
  }

  void _reverse_array(uint8_t* array, size_t size) {
    for (size_t i = 0; i < size / 2; i++) {
      uint8_t temp        = array[i];
      array[i]            = array[size - 1 - i];
      array[size - 1 - i] = temp;
    }
  }

  status_t _transfer(uint8_t* data, uint8_t addr, size_t size) {
    _spi->beginTransaction(_spiSettings);
    digitalWrite(_pinCS, LOW);
    _status.raw = _spi->transfer(addr);
    if (size) _spi->transfer(data, size);
    digitalWrite(_pinCS, HIGH);
    _spi->endTransaction();

#ifdef MCP3x6x_DEBUG
    MCP3x6x_LOG(addr);
    MCP3x6x_LOG(":");
    for (size_t i = 0; i < size; i++) {
      MCP3x6x_LOG("0x");
      MCP3x6x_LOG(data[i], HEX);
      MCP3x6x_LOG(", ")
    }
    MCP3x6x_LOG("\r\n");
#endif

    return _status;
  }

  //  status_t _transfer16(uint8_t *data, uint8_t addr);
  int32_t _getValue(int32_t raw) {
    switch (_config3.data_format) {
      case data_format::SGN_DATA_ZERO:
        return raw >> (_MAX_RESOLUTION == 16 ? 16 : 8);
      case data_format::SGN_DATA:
        return (int32_t)(raw << (_MAX_RESOLUTION == 16 ? 16 : 8)) >>
               (_MAX_RESOLUTION == 16 ? 16 : 8);
      case data_format::SGNEXT_DATA:
      case data_format::ID_SGNEXT_DATA:
        return raw;
    }
    return -1;
  }
  int8_t _getChannel(uint32_t raw) {
    if (_config3.data_format == data_format::ID_SGNEXT_DATA) {
      return ((raw >> 28) & 0x0F);
    } else {
      for (size_t i = 0; i < sizeof(_channelID) / sizeof(_channelID[0]); i++) {
        if (_channelID[i] == _mux.raw) {
          return i;
        }
      }
    }
    return -1;
  }

  /**
   * @brief number of bytes to read for ADCDATA based on resolution and format
   *
   * @return size_t number of bytes
   */
  size_t _adcReadSize() const {
    switch (_MAX_RESOLUTION) {
      case 16:
        return _config3.data_format == data_format::SGN_DATA ? 2 : 4;
      case 24:
        return _config3.data_format == data_format::SGN_DATA ? 3 : 4;
    }
    return 4;
  }

  volatile bool _dataAvailable = false;
  float _reference             = 2.4;
  uint32_t _readTimeout        = 0;
  int16_t _tempOffset          = 0;
  uint16_t _tempSlope          = 1150;
  uint8_t _pinCS, _pinMISO, _pinMOSI, _pinCLK;  // SPI pins
  uint8_t _pinIRQ, _pinMCLK;

  ///////////////////////////////////////////////////////////////////////////////
  // DMA
  ////////////////////////////////////////////////////////////////////////////////

  // payload staging: engines clock the raw ADCDATA bytes here (wire order)
  uint8_t _dmaData[4] = {0};
  DmaRingBuffer _dmaRing;
  bool _dmaEnabled    = false;
  uint8_t _dmaChannel = 0;

#ifdef ARDUINO_ARCH_ESP32
  spi_device_handle_t _dmaEsp32Handle = nullptr;
  spi_transaction_t _dmaEsp32Trans    = {};
#endif
#ifdef ARDUINO_ARCH_SAMD
  uint8_t _dmaSamdChan = 0;
  bool _dmaSamdActive  = false;
  // DMAC descriptor (16-byte aligned, in SRAM)
  struct __attribute__((aligned(16))) DmacDescriptor {
    uint16_t BTCTRL;
    uint8_t reserved[2];
    uint32_t SRCADDR;
    uint32_t DSTADDR;
    uint32_t DESCADDR;
  };
  DmacDescriptor _dmaSamdDescriptor;
#endif

  /**
   * @brief decode a completed DMA transfer into a sample and push it to the ring
   *
   * Mirrors readADCDATA() decode: big-endian wire bytes, _getValue() conversion,
   * ID_SGNEXT_DATA channel nibble strip + sign extension.
   */
  void _dmaComplete(uint32_t timestamp) {
    uint8_t buffer[4];
    memcpy(buffer, _dmaData, 4);
    _reverse_array(buffer, 4);
    int32_t value   = _getValue((uint32_t&)buffer);
    uint32_t raw    = (uint32_t)value;
    uint8_t channel = (uint8_t)_getChannel(raw);
    if (_config3.data_format == data_format::ID_SGNEXT_DATA) {
      value = (int32_t)((uint32_t)raw << 4) >> 4;
    }
    DmaSample s;
    s.channel   = channel;
    s.value     = value;
    s.timestamp = timestamp;
    _dmaRing.push(s);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // DMA platform engines
  ///////////////////////////////////////////////////////////////////////////////

  /**
   * @brief initialize the platform DMA engine; false = not supported / conflict
   */
  bool _dmaInitEngine(uint8_t dmaChannel) {
#ifdef PIO_NATIVE_TESTING
    // native tests use a mock engine so ring/lifecycle logic is testable on host
    (void)dmaChannel;
    return true;
#elif defined(ARDUINO_ARCH_ESP32)
    return _dmaInitESP32(dmaChannel);
#elif defined(ARDUINO_ARCH_SAMD)
    return _dmaInitSAMD(dmaChannel);
#else
    (void)dmaChannel;
    return false;
#endif
  }

  void _dmaStartEngine() {
#ifdef PIO_NATIVE_TESTING
      // no-op: native tests push directly via _dmaComplete()
#elif defined(ARDUINO_ARCH_ESP32)
    _dmaStartESP32();
#elif defined(ARDUINO_ARCH_SAMD)
    _dmaStartSAMD();
#endif
  }

  /**
   * @brief pull finished transactions from the engine (only ESP32 needs polling)
   */
  void _dmaPollEngine() {
#ifdef ARDUINO_ARCH_ESP32
    _dmaPollESP32();
#elif defined(ARDUINO_ARCH_SAMD)
    _dmaPollSAMD();
#endif
  }

  void _dmaStopEngine() {
#ifdef PIO_NATIVE_TESTING
      // mock engine: nothing to release
#elif defined(ARDUINO_ARCH_ESP32)
    _dmaStopESP32();
#elif defined(ARDUINO_ARCH_SAMD)
    _dmaStopSAMD();
#endif
  }

  ///////////////////////////////////////////////////////////////////////////////
  // ESP32 DMA engine: ESP-IDF spi_master driver
  ///////////////////////////////////////////////////////////////////////////////
#ifdef ARDUINO_ARCH_ESP32
  bool _dmaInitESP32(uint8_t dmaChannel) {
      // ESP32-C3 has only 2 SPI hosts; ESP32/ESP32-S2/S3 have 3
  #if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32C3)
    spi_host_device_t host = (dmaChannel == 1) ? SPI3_HOST : SPI2_HOST;
  #else
    spi_host_device_t host = SPI2_HOST;  // ESP32-C3: only SPI2_HOST available
  #endif
    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz                = 1000000;  // 1 MHz: safe default for ADCDATA reads
    devcfg.mode                          = 0;
    devcfg.spics_io_num                  = (int)_pinCS;
    devcfg.queue_size                    = 1;
    devcfg.flags                         = 0;
    esp_err_t err                        = spi_bus_add_device(host, &devcfg, &_dmaEsp32Handle);
    return (err == ESP_OK);
  }

  void _dmaStartESP32() {
    if (!_dmaEsp32Handle) {
      return;
    }
    uint8_t addr             = MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA;
    _dmaEsp32Trans           = {};
    _dmaEsp32Trans.length    = 8;   // 1 byte: address/command
    _dmaEsp32Trans.rxlength  = 32;  // 4 bytes: ADCDATA payload
    _dmaEsp32Trans.tx_buffer = &addr;
    _dmaEsp32Trans.rx_buffer = _dmaData;
    _dmaEsp32Trans.flags     = 0;
    spi_device_queue_trans(_dmaEsp32Handle, &_dmaEsp32Trans, portMAX_DELAY);
  }

  void _dmaPollESP32() {
    if (!_dmaEsp32Handle) {
      return;
    }
    spi_transaction_t* trans = nullptr;
    esp_err_t err            = spi_device_get_trans_result(_dmaEsp32Handle, &trans, 0);
    if (err == ESP_OK && trans) {
      _dmaComplete(micros());
      // Re-arm for the next conversion: queue another transaction
      _dmaStartESP32();
    }
  }

  void _dmaStopESP32() {
    if (_dmaEsp32Handle) {
      spi_bus_remove_device(_dmaEsp32Handle);
      _dmaEsp32Handle = nullptr;
    }
    _dmaEsp32Trans = {};
  }
#endif  // ARDUINO_ARCH_ESP32

  ///////////////////////////////////////////////////////////////////////////////
  // SAMD DMA engine: SERCOM DMAC RX trigger
  ///////////////////////////////////////////////////////////////////////////////
#ifdef ARDUINO_ARCH_SAMD
  bool _dmaInitSAMD(uint8_t dmaChannel) {
  #if defined(__SAMD21__)
    if (dmaChannel > 7) return false;
  #elif defined(__SAMD51__)
    if (dmaChannel > 11) return false;
  #else
    return false;
  #endif
    _dmaSamdChan   = dmaChannel;
    _dmaSamdActive = true;
    return true;
  }

  void _dmaStartSAMD() {
    if (!_dmaSamdActive) return;

    // Get SERCOM pointer from SPI class (public API)
    uint8_t sercomIndex = _spi->getSercomIndex();
    Sercom* spiSercom   = reinterpret_cast<Sercom*>(0x42000000UL + sercomIndex * 0x400UL);
    if (!spiSercom) return;

  #if defined(__SAMD21__)
    // SAMD21: select channel via CHID, then access channel registers directly
    DMAC->CHID.reg = _dmaSamdChan;
    DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
    while (DMAC->CHCTRLA.bit.ENABLE) {
      ;
    }
    DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
    while (DMAC->CHCTRLA.bit.SWRST) {
      ;
    }

    // Set up descriptor
    _dmaSamdDescriptor.BTCTRL =
        DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_SRCINC | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_VALID;
    _dmaSamdDescriptor.SRCADDR  = (uint32_t)&(spiSercom->SPI.DATA.reg);
    _dmaSamdDescriptor.DSTADDR  = (uint32_t)_dmaData;
    _dmaSamdDescriptor.DESCADDR = 0;

    DMAC->BASEADDR.reg          = (uint32_t)&_dmaSamdDescriptor;

    DMAC->CHCTRLB.reg = DMAC_CHCTRLB_LVL(0) | DMAC_CHCTRLB_EVOE | DMAC_CHCTRLB_EVACT_TRIG |
                        DMAC_CHCTRLB_TRIGSRC(_spi->getDMAC_ID_RX());

    DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
    while (!DMAC->CHCTRLA.bit.ENABLE) {
      ;
    }

    DMAC->CHINTENSET.reg = DMAC_CHINTENSET_TCMPL;

  #elif defined(__SAMD51__)
    // SAMD51: channel registers accessed via DMAC->Channel[n] array
    DmacChannel* ch = &DMAC->Channel[_dmaSamdChan];

    // Disable channel during configuration
    ch->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
    while (ch->CHCTRLA.bit.ENABLE) {
      ;
    }

    // Software reset
    ch->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
    while (ch->CHCTRLA.bit.SWRST) {
      ;
    }

    // Set up descriptor (SAMD51 descriptor has BTCNT field at offset 2)
    DMAC_BTCTRL_Type btctrl;
    btctrl.reg =
        DMAC_BTCTRL_BEATSIZE_BYTE | DMAC_BTCTRL_SRCINC | DMAC_BTCTRL_DSTINC | DMAC_BTCTRL_VALID;
    _dmaSamdDescriptor.BTCTRL   = btctrl.reg;
    _dmaSamdDescriptor.SRCADDR  = (uint32_t)&(spiSercom->SPI.DATA.reg);
    _dmaSamdDescriptor.DSTADDR  = (uint32_t)_dmaData;
    _dmaSamdDescriptor.DESCADDR = 0;

    DMAC->BASEADDR.reg          = (uint32_t)&_dmaSamdDescriptor;

    // Configure channel control A: trigger source + trigger action
    ch->CHCTRLA.reg =
        DMAC_CHCTRLA_TRIGSRC(_spi->getDMAC_ID_RX()) | DMAC_CHCTRLA_TRIGACT_TRANSACTION;

    // Enable channel
    ch->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
    while (!ch->CHCTRLA.bit.ENABLE) {
      ;
    }

    // Enable transfer complete interrupt
    ch->CHINTENSET.reg = DMAC_CHINTENSET_TCMPL;
  #endif
  }

  void _dmaPollSAMD() {
    if (!_dmaSamdActive) return;
  #if defined(__SAMD21__)
    if (DMAC->CHINTFLAG.reg & DMAC_CHINTFLAG_TCMPL) {
      DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
      _dmaComplete(micros());
      _dmaStartSAMD();
    }
  #elif defined(__SAMD51__)
    if (DMAC->Channel[_dmaSamdChan].CHINTFLAG.reg & DMAC_CHINTFLAG_TCMPL) {
      DMAC->Channel[_dmaSamdChan].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
      _dmaComplete(micros());
      _dmaStartSAMD();
    }
  #endif
  }

  void _dmaStopSAMD() {
    if (!_dmaSamdActive) return;
  #if defined(__SAMD21__)
    DMAC->CHID.reg = _dmaSamdChan;
    DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
    while (DMAC->CHCTRLA.bit.ENABLE) {
      ;
    }
    DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL | DMAC_CHINTFLAG_SUSP;
  #elif defined(__SAMD51__)
    DMAC->Channel[_dmaSamdChan].CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
    while (DMAC->Channel[_dmaSamdChan].CHCTRLA.bit.ENABLE) {
      ;
    }
    DMAC->Channel[_dmaSamdChan].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL | DMAC_CHINTFLAG_SUSP;
  #endif
    _dmaSamdActive = false;
  }
#endif  // ARDUINO_ARCH_SAMD

  SPIClass* _spi;            // SPI interface
  SPISettings _spiSettings;  // SPI settings

  const uint8_t _channelID[16] = {MCP3x6x_CH0,   MCP3x6x_CH1,   MCP3x6x_CH2,   MCP3x6x_CH3,
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
      enum adc_mode adc : 2;  //!< ADC Operating Mode Selection
      enum cs_sel bias : 2;   //!< Current Source/Sink Selection Bits for Sensor Bias
      enum clk_sel clk : 2;   //!< Clock Selection
      uint8_t cfg0  : 1;      //!< Full Shutdown Mode Enable (bit 6)
      bool vref_sel : 1;      //!< Internal Voltage Reference Selection (bit 7)
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
      uint8_t : 2;       //!< reserved
      enum osr osr : 4;  //!< Oversampling Ratio for Delta-Sigma A/D Conversion
      enum pre pre : 2;  //!< Prescaler Value Selection for AMCLK
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
      bool reserved : 1;     //!< reserved, must always be '1' (bit 0)
      bool az_ref   : 1;     //!< Auto-Zeroing Reference Buffer Setting (bit 1)
      bool az_mux   : 1;     //!< Auto-Zeroing MUX Setting (bit 2)
      enum gain gain : 3;    //!< ADC Gain Selection (bits 5:3)
      enum boost boost : 2;  //!< ADC Bias Current Selection (bits 7:6)
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
      bool en_gaincal : 1;               //!< Enable Digital Gain Calibration
      bool en_offcal  : 1;               //!< Enable Digital Offset Calibration
      bool en_crccom  : 1;               //!< CRC Checksum Selection on Read Communications
      bool crc_format : 1;               //!< CRC Checksum Format Selection on Read
                                         //!< Communications
      enum data_format data_format : 2;  //!< ADC Output Data Format Selection
      enum conv_mode conv_mode : 2;      //!< Conversion Mode Selection
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
      bool en_stp      : 1;  //!< Enable Conversion Start Interrupt Output
      bool en_fastcmd  : 1;  //!< Enable Fast Commands in the COMMAND Byte
      uint8_t irq_mode : 2;  //!< Configuration for the IRQ/MDAT Pin
      bool             : 1;  //!< reserved (POR_STATUS, read-only)
      bool             : 1;  //!< reserved (CRCCFG_STATUS, read-only)
      bool             : 1;  //!< reserved (DR_STATUS, read-only)
      bool             : 1;  //!< unimplemented
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
      enum mux vin_minus : 4;  //!< MUX_VIN- Input Selection
      enum mux vin_plus : 4;   //!< MUX_VIN+ Input Selection
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
      enum delay dly : 3;         //!< delay time between each conversion during a scan cycle
    };
    uint8_t raw[3];  //!< raw access to register
    Scan(const uint8_t data[3] = MCP3x6x_CFG_SCAN) : raw{data[0], data[1], data[2]} {}
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
    Timer(const uint8_t data[3] = MCP3x6x_CFG_TIMER) : raw{data[0], data[1], data[2]} {}
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
    Offset(const uint8_t data[3] = MCP3x6x_CFG_OFFSET) : raw{data[0], data[1], data[2]} {}
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
    Gain(const uint8_t data[3] = MCP3x6x_CFG_GAIN) : raw{data[0], data[1], data[2]} {}
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
    uint8_t channelid : 4;                              //!< channel ID
    int32_t value     : 25;                             //!< actual value of conversion
  } _adcdata = {.channelid = MCP3x6x_CH0, .value = 0};  //!< todo

 public:
  ///////////////////////////////////////////////////////////////////////////////
  // de/constructor
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Construct a new MCP3x6x object
   *
   * @param pinCS       SPI chip-select pin (default SS)
   * @param pinMOSI     SPI MOSI pin (default MOSI)
   * @param pinMISO     SPI MISO pin (default MISO)
   * @param pinCLK      SPI clock pin (default SCK)
   * @param theSPI      SPI bus interface (default &SPI)
   * @param theSPISettings SPI transaction settings (default SPISettings())
   *
   * @ingroup init
   */
  explicit MCP3x6x(uint8_t pinCS = SS, uint8_t pinMOSI = MOSI, uint8_t pinMISO = MISO,
                   uint8_t pinCLK = SCK, SPIClass* theSPI = &SPI,
                   SPISettings theSPISettings = SPISettings())
      : _result{},
        _pinCS(pinCS),
        _pinMISO(pinMISO),
        _pinMOSI(pinMOSI),
        _pinCLK(pinCLK),
        _pinIRQ(0),
        _pinMCLK(0),
        _spi(theSPI),
        _spiSettings(theSPISettings),
        _config0(MCP3x6x_CFG_CONFIG0),
        _config1(MCP3x6x_CFG_CONFIG1),
        _config2(MCP3x6x_CFG_CONFIG2),
        _config3(MCP3x6x_CFG_CONFIG3),
        _irq(MCP3x6x_CFG_IRQ),
        _mux(MCP3x6x_CFG_MUX),
        _scan(MCP3x6x_CFG_SCAN),
        _timer(MCP3x6x_CFG_TIMER),
        _offset(MCP3x6x_CFG_OFFSET),
        _gain(MCP3x6x_CFG_GAIN),
        _lock(MCP3x6x_CFG_LOCK),
        _crccfg(MCP3x6x_CFG_CRCCFG) {}

  /**
   * @brief Destroy the MCP3x6x object
   *
   */
  ~MCP3x6x() {
    if (_dmaEnabled) disableDMA();
    if (_pinIRQ) detachInterrupt(digitalPinToInterrupt(_pinIRQ));
    if (_pinMCLK) noTone(_pinMCLK);
    end();
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  ////////////////////////////////////////////////////////////////////////////////

  // setup SPI
  bool begin() {
#if defined(ARDUINO_ARCH_ESP32)  // init SPI of ESP32
    _spi->begin(_pinCS, _pinMISO, _pinMOSI, _pinCLK);

#elif defined(ARDUINO_ARCH_STM32)  // init SPI of STM32
    _spi = new SPIClass(_pinCS, _pinMISO, _pinMOSI, _pinCLK);
    _spi->begin();

#elif defined(ARDUINO_ARCH_ESP8266)  // init SPI of ESP8266
    if (!_spi->pins(_pinCLK, _pinMISO, _pinMOSI, _pinCS)) {
      // Ensure CS is configured even if pin remapping fails
      pinMode(_pinCS, OUTPUT);
      digitalWrite(_pinCS, HIGH);
    } else {
      pinMode(_pinCS, OUTPUT);
      digitalWrite(_pinCS, HIGH);
    }
    _spi->begin();

#else  // default init SPI
    pinMode(_pinCS, OUTPUT);
    digitalWrite(_pinCS, HIGH);
    _spi->begin();

  #if defined(ARDUINO_ARCH_SAMD)
    pinPeripheral(_pinMISO, PIO_SERCOM);
    pinPeripheral(_pinMOSI, PIO_SERCOM);
    pinPeripheral(_pinCLK, PIO_SERCOM);
  #endif
#endif

    // reset ADC and return
    if (!reset().por) {
      return false;
    }

    // chip ID validation
    if (!_validateChipID()) {
      return false;
    }

    // R-variant detection and VREF auto-configuration
    _detectVariant();

    return true;
  }

  bool _validateChipID() {
    uint8_t reserved_e = 0;
    _transfer(&reserved_e, MCP3x6x_CMD_SREAD | MCP3x6x_ADR_RESERVED3, sizeof(reserved_e));
    uint8_t hw_id = reserved_e & 0x0F;

    uint8_t expected_id;
    if (_MAX_RESOLUTION == 16) {
      if (_MAX_CHANNELS == 2)
        expected_id = 0x8;
      else if (_MAX_CHANNELS == 4)
        expected_id = 0x9;
      else
        expected_id = 0xB;
    } else {
      if (_MAX_CHANNELS == 2)
        expected_id = 0xC;
      else if (_MAX_CHANNELS == 4)
        expected_id = 0xD;
      else
        expected_id = 0xF;
    }

    if (hw_id != expected_id) {
      MCP3x6x_LOG("MCP3x6x: chip ID mismatch\n");
      return false;
    }
    return true;
  }

  void _detectVariant() {
    uint8_t reserved_c = 0;
    _transfer(&reserved_c, MCP3x6x_CMD_SREAD | MCP3x6x_ADR_RESERVED2, sizeof(reserved_c));

    if (reserved_c == 0x30) {
      _config0.vref_sel = 1;
      _reference        = 2.4f;
      write(_config0);
    } else {
      _config0.vref_sel = 0;
      _reference        = 0.0f;
    }
  }

  /**
   * @brief end communication
   *
   * @ingroup init
   */
  void end() { _spi->end(); }

  void configure(Config0 config0, Config1 config1, Config2 config2, Config3 config3, Irq irq,
                 Mux mux, Scan scan, Timer timer, Offset offset, Gain gain, Lock lock,
                 Crccfg crccfg) {
    unlock();

    if (_config0.raw != config0.raw) {
      write(_config0 = config0);
    }

    if (_config1.raw != config1.raw) {
      write(_config1 = config1);
    }

    if (_config2.raw != config2.raw) {
      write(_config2 = config2);
    }

    if (_config3.raw != config3.raw) {
      write(_config3 = config3);
    }

    if (_irq.raw != irq.raw) {
      write(_irq = irq);
    }

    if (_mux.raw != mux.raw) {
      write(_mux = mux);
    }

    if (memcmp(_scan.raw, scan.raw, sizeof(scan.raw))) {
      write(_scan = scan);
    }

    if (memcmp(_timer.raw, timer.raw, sizeof(timer.raw))) {
      write(_timer = timer);
    }

    if (memcmp(_offset.raw, offset.raw, sizeof(offset.raw))) {
      write(_offset = offset);
    }

    if (memcmp(_gain.raw, gain.raw, sizeof(gain.raw))) {
      write(_gain = gain);
    }

    if (memcmp(_crccfg.raw, crccfg.raw, sizeof(crccfg))) {
      write(_crccfg = crccfg);
    }

    _lock = lock;
    write(_lock);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // attach pins
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief attaches pin to interrupt callback function
   *
   * @param pinIRQ    IRQ input pin number
   * @param callback  interrupt service routine called on falling edge
   *
   * @ingroup pins
   */
  void attachIRQ(const uint8_t pinIRQ, void (*callback)(void)) {
    _pinIRQ = pinIRQ;

    attachInterrupt(digitalPinToInterrupt(_pinIRQ), callback, FALLING);
  }

  /**
   * @brief attaches pin to clock generator
   *
   * @param pinMCLK   MCLK output pin number (generates 4.9152 MHz master clock)
   *
   * @ingroup pins
   */
  void attachMCLK(const uint8_t pinMCLK) {
    _pinMCLK = pinMCLK;

#if ((F_CPU / 2) < 4915200)
// #  error "MCLK frequency is too low"
#else
    tone(_pinMCLK, 4915200);
#endif
  }

  ///////////////////////////////////////////////////////////////////////////////
  // set configuration registers
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief write register CONFIG0 to device
   *
   * @param adc      ADC operating mode (CONVERSION, STANDBY, SHUTDOWN)
   * @param bias     Sensor bias current source selection
   * @param clk      Clock source selection
   * @param vref_sel Enable internal voltage reference
   * @param cfg0     Config0 raw bit (default 0)
   *
   * @ingroup config
   */
  void config0(enum adc_mode adc, enum cs_sel bias, enum clk_sel clk, bool vref_sel,
               bool cfg0 = 0) {
    _config0.adc      = adc;
    _config0.bias     = bias;
    _config0.clk      = clk;
    _config0.vref_sel = vref_sel;
    _config0.cfg0     = cfg0;

    write(_config0);
  }

  /**
   * @brief write register CONFIG1 to device
   *
   * @param osr  Oversampling ratio for delta-sigma conversion
   * @param pre  Prescaler value selection for AMCLK
   *
   * @ingroup config
   */
  void config1(enum osr osr, enum pre pre) {
    _config1.osr = osr;
    _config1.pre = pre;
    write(_config1);
  }

  /**
   * @brief write register CONFIG2 to device
   *
   * @param az_mux  Auto-zero mux enable
   * @param gain    PGA gain selection
   * @param boost   Boost mode selection
   * @param az_ref  Auto-zero reference enable (default false)
   *
   * @ingroup config
   */
  void config2(bool az_mux, enum gain gain, enum boost boost, bool az_ref = false) {
    _config2.az_mux = az_mux;
    _config2.gain   = gain;
    _config2.boost  = boost;
    _config2.az_ref = az_ref;
    write(_config2);
  }

  /**
   * @brief write register CONFIG3 to device
   *
   * @param gaincal    Enable gain calibration
   * @param offcal     Enable offset calibration
   * @param crccom     Enable CRC on communications
   * @param data_format ADC output data format selection
   * @param conv_mode  Conversion mode (continuous, one-shot standby, one-shot shutdown)
   *
   * @ingroup config
   */
  void config3(bool gaincal, bool offcal, bool crccom, enum data_format data_format,
               enum conv_mode conv_mode) {
    _config3.en_gaincal  = gaincal;
    _config3.en_offcal   = offcal;
    _config3.en_crccom   = crccom;
    _config3.data_format = data_format;
    _config3.conv_mode   = conv_mode;
    write(_config3);
  }

  /**
   * @brief write register IRQ to device
   *
   * @param stp      Stop conversion on IRQ
   * @param fastcmd  Enable fast command mode
   * @param mode     IRQ mode selection
   *
   * @ingroup config
   */
  void irq(bool stp, bool fastcmd, enum irq_mode mode) {
    _irq.en_stp     = stp;
    _irq.en_fastcmd = fastcmd;
    _irq.irq_mode   = (uint8_t)mode;
    write(_irq);
  }

  /**
   * @brief write register MUX to device
   *
   * @param minus  Negative input channel selection
   * @param plus   Positive input channel selection
   *
   * @ingroup config
   */
  void mux(enum mux minus, enum mux plus) {
    _mux.vin_minus = minus;
    _mux.vin_plus  = plus;
    write(_mux);
  }

  /**
   * @brief write register SCAN to device
   *
   * @param single_ended  Bitmask of single-ended channels
   * @param differential  Bitmask of differential channel pairs
   * @param temp          Include temperature sensor in scan
   * @param avdd          Include AVDD in scan
   * @param vcm           Include VCM in scan
   * @param offset        Include offset calibration in scan
   * @param dly           Delay between conversions in scan cycle
   *
   * @ingroup config
   * @see startScan(), startScanOnce(), enableScanChannel(), disableScanChannel()
   */
  void scan(byte single_ended, byte differential, bool temp, bool avdd, bool vcm, bool offset,
            enum delay dly) {
    _scan.channel.single_ended = single_ended;
    _scan.channel.differential = differential;
    _scan.channel.temp         = temp;
    _scan.channel.avdd         = avdd;
    _scan.channel.vcm          = vcm;
    _scan.channel.offset       = offset;
    _scan.dly                  = dly;
    write(_scan);
  }

  /**
   * @brief write register TIMER to device
   *
   * @param timer  Pointer to 3-byte timer value
   *
   * @ingroup config
   */
  void timer(uint8_t* timer) {
    _timer = timer;
    write(_timer);
  }

  /**
   * @brief write register OFFSET to device
   *
   * @param offset  Pointer to 3-byte offset calibration value
   *
   * @ingroup calibration
   */
  void offset(uint8_t* offset) {
    _offset = offset;
    write(_offset);
  }

  /**
   * @brief write register GAIN to device
   *
   * @param gain  Pointer to 3-byte gain calibration value
   *
   * @ingroup calibration
   */
  void gain(uint8_t* gain) {
    _gain = gain;
    write(_gain);
  }

  /**
   * @brief write register LOCK to device (key: 0xA5 unlocks the register map,
   *        any other value locks it)
   *
   * @param key  lock key / lock value (default 0x00 = locked)
   * @return status_t
   *
   * @ingroup config
   * @warning Writing any value other than 0xA5 locks the register map until
   *          the correct unlock key is written.
   */
  status_t lock(uint8_t key = 0x00) {
    _lock.raw = key;
    return write(_lock);
  }

  /**
   * @brief write register LOCK to unlock device (uses the datasheet key 0xA5)
   *
   * @return status_t
   *
   * @ingroup config
   * @see lock()
   */
  status_t unlock() { return lock(MCP3x6x_CFG_LOCK); }

  /**
   * @brief write register CRCCFG to device
   *
   * @param crccfg  CRC configuration value
   *
   * @ingroup config
   */
  void crccfg(uint16_t crccfg) {
    _crccfg.value = crccfg;
    write(_crccfg);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // read status
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief data ready status of latest communication
   *
   * @return true  data is ready
   * @return false  data is not ready
   *
   * @ingroup status
   */
  inline constexpr bool status_dr() { return !_status.dr; }

  /**
   * @brief crccfg status of latest communication
   *
   * @return true  CRC configuration is valid
   * @return false  CRC configuration is invalid
   *
   * @ingroup status
   */
  inline constexpr bool status_crccfg() { return !_status.crccfg; }

  /**
   * @brief power on reset status of latest communication
   *
   * @return true  POR occurred
   * @return false  no POR
   *
   * @ingroup status
   */
  inline constexpr bool status_por() { return !_status.por; }

  ///////////////////////////////////////////////////////////////////////////////
  // fast commands
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Fast Command — Start conversion
   *
   * @ingroup conversion
   * @return status_t
   */
  inline status_t conversion() { return _fastcmd(MCP3x6x_CMD_CONVERSION); }

  /**
   * @brief Fast Command — Enter standby mode
   *
   * @ingroup lowpower
   * @return status_t
   */
  inline status_t standby() { return _fastcmd(MCP3x6x_CMD_STANDBY); }

  /**
   * @brief Fast Command — Enter shutdown mode
   *
   * @ingroup lowpower
   * @return status_t
   */
  inline status_t shutdown() { return _fastcmd(MCP3x6x_CMD_SHUTDOWN); }

  /**
   * @brief Fast Command — Full Shutdown (lowest power)
   *
   * @ingroup lowpower
   * @warning Datasheet errata: AVDD and DVDD must drop below 100 mV before
   *          power is reapplied after Full Shutdown, otherwise CHIPID bits
   *          may be corrupted.
   * @return status_t
   */
  inline status_t full_shutdown() { return _fastcmd(MCP3x6x_CMD_FULL_SHUTDOWN); }

  /**
   * @brief Fast Command — Reset ADC
   *
   * @ingroup init
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
    data.reserved = 1;  // datasheet requires bit 0 always = 1
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
    data.reserved = 0;  // datasheet requires bit 20 = 0
    _reverse_array(data.raw, 3);
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, 3);
  }

  /**
   * @brief write TIMER to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Timer data) {
    _reverse_array(data.raw, 3);
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_TIMER, 3);
  }

  /**
   * @brief write OFFSET to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Offset data) {
    _reverse_array(data.raw, 3);
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_OFFSET, 3);
  }

  /**
   * @brief write GAIN to ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t write(Gain data) {
    _reverse_array(data.raw, 3);
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
    _reverse_array(data.raw, 2);
    return _transfer(data.raw, MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CRCCFG, 2);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // read registers
  ////////////////////////////////////////////////////////////////////////////////

  inline byte status() { return _status.raw; }

  /**
   * @brief read register ADCDATA from ADC
   *
   * @param data
   * @return status_t
   */
  status_t read(Adcdata* data) {
    size_t s = _adcReadSize();
    uint8_t buffer[4];
    memset(buffer, 0, 4);
    _transfer(buffer, MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, s);
    _reverse_array(buffer, s);
    data->channelid = _getChannel((uint32_t&)buffer);
    data->value     = _getValue((uint32_t&)buffer);
    if (_config3.data_format == data_format::ID_SGNEXT_DATA) {
      data->value = (int32_t)((uint32_t)data->value << 4) >> 4;
    }
    return _status;
  }

  /**
   * @brief read register CONFIG0 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Config0& data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, 1);
  }

  /**
   * @brief read register CONFIG1 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Config1& data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1, 1);
  }

  /**
   * @brief read register CONFIG2 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Config2& data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG2, 1);
  }

  /**
   * @brief read register CONFIG3 from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Config3& data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG3, 1);
  }

  /**
   * @brief read register IRQ from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Irq& data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_IRQ, 1);
  }

  /**
   * @brief read register MUX from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Mux& data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX, 1);
  }

  /**
   * @brief read register SCAN from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Scan& data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_SCAN, 3);
  }

  /**
   * @brief read register TIMER from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Timer& data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_TIMER, 3);
  }

  /**
   * @brief read register OFFSET from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Offset& data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_OFFSET, 3);
  }

  /**
   * @brief read register GAIN from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Gain& data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_GAIN, 3);
  }

  /**
   * @brief read register LOCK from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Lock& data) {
    return _transfer(&data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_LOCK, 1);
  }

  /**
   * @brief read register CRCCFG from ADC
   *
   * @param data
   * @return status_t
   */
  inline status_t read(Crccfg& data) {
    return _transfer(data.raw, MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CRCCFG, 2);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // data access
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief report whether a new conversion result is available
   *
   * @return true  new data available since last read
   * @return false  no new data
   *
   * @ingroup status
   */
  bool dataAvailable() { return _dataAvailable; }

  /**
   * @brief read raw ADC data from the last conversion
   *
   * @return int32_t sign-extended ADC value
   *
   * @ingroup conversion
   * @see dataAvailable(), getLastChannel(), toVoltage()
   */
  int32_t readADCDATA() {
    size_t s          = _adcReadSize();
    uint8_t buffer[4] = {0};
    _transfer(buffer, MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, s);
    _reverse_array(buffer, s);
    _dataAvailable  = false;
    int32_t value   = _getValue((uint32_t&)buffer);
    uint32_t raw    = (uint32_t)value;
    uint8_t channel = (uint8_t)_getChannel(raw);
    if (_config3.data_format == data_format::ID_SGNEXT_DATA) {
      value = (int32_t)((uint32_t)raw << 4) >> 4;
    }
    _adcdata.value     = value;
    _adcdata.channelid = channel;
    return value;
  }

  /**
   * @brief get channel of the last ADC read result
   *
   * @return int8_t channel ID
   */
  int8_t getLastChannel() { return _adcdata.channelid; }

  ///////////////////////////////////////////////////////////////////////////////
  // DMA API (scan mode only; mux mode stays synchronous)
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief enable DMA-backed scan sampling
   *
   * Registers the platform DMA engine and prepares the ring buffer. DMA is
   * rejected when capacity < 1, when the platform has no DMA-capable SPI
   * engine (AVR, ESP8266), when the selected channel conflicts with MCLK, or
   * when the engine fails to initialize.
   *
   * @param buffer user-provided sample storage (DMA writes here, ring holds
   *               capacity*2 x enabled channels recommended)
   * @param capacity ring capacity (>= 1)
   * @param dmaChannel platform channel selector (0 = default; on ESP32 this
   *        selects the SPI host: 0 = SPI2_HOST, 1 = SPI3_HOST)
   * @return true DMA enabled; subsequent scan-mode conversions land in buffer
   * @return false DMA not enabled; scan path stays synchronous
   */
  bool enableDMA(DmaSample* buffer, size_t capacity, uint8_t dmaChannel = 0) {
    if (capacity < 1) {
      return false;
    }
    if (_dmaEnabled) {
      return false;
    }
    if (!_dmaInitEngine(dmaChannel)) {
      return false;
    }
    _dmaRing.setup(buffer, capacity);
    _dmaChannel = dmaChannel;
    _dmaEnabled = true;
    return true;
  }

  /**
   * @brief get pointer to DMA staging buffer (for test access)
   *
   * @return uint8_t* pointer to the 4-byte ADCDATA staging buffer
   */
  uint8_t* dmaDataPtr() { return _dmaData; }

  /**
   * @brief disable DMA sampling and release platform resources (MCLK tone off)
   */
  void disableDMA() {
    if (!_dmaEnabled) {
      return;
    }
    _dmaEnabled = false;
    _dmaRing.clear();
    _dmaStopEngine();
    if (_pinMCLK) noTone(_pinMCLK);
  }

  /**
   * @brief report whether DMA sampling is enabled
   */
  bool isDMAEnabled() const { return _dmaEnabled; }

  /**
   * @brief number of DMA samples currently stored (0 when DMA disabled)
   */
  size_t samplesAvailable() {
    if (!_dmaEnabled) {
      return 0;
    }
    _dmaPollEngine();  // ESP32: pull completed transactions first
    return _dmaRing.available();
  }

  /**
   * @brief pop the oldest DMA sample
   *
   * @param sample receives the oldest stored sample
   * @return true sample was returned
   * @return false DMA disabled or no samples stored
   */
  bool readSample(DmaSample& sample) {
    if (!_dmaEnabled) {
      return false;
    }
    _dmaPollEngine();  // ESP32: pull completed transactions first
    return _dmaRing.pop(sample);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief interrupt request handler
   *
   * Call this from your ISR when the ADC IRQ fires. In DMA+scan mode it starts
   * the DMA engine; otherwise it reads ADCDATA and marks data as available.
   *
   * @ingroup status
   * @see dataAvailable(), readADCDATA(), attachIRQ()
   */
  void IRQ_handler() {
    // DMA path: scan mode with DMA enabled reads ADCDATA non-blocking
    if (_dmaEnabled && _scan.channel.raw != 0) {
      _dmaStartEngine();
      _dataAvailable = true;
      return;
    }
    // Read the ADC data when IRQ is triggered
    read(&_adcdata);
    _result.raw[(uint8_t)_adcdata.channelid] = _adcdata.value;
    _dataAvailable                           = true;
  }

  /**
   * @brief Set the Data Format
   *
   * @param format  ADC output data format (see data_format enum)
   * @return status_t
   *
   * @ingroup config
   * @see config3()
   */
  status_t setDataFormat(data_format format) {
    _config3.data_format = format;
    return write(_config3);
  }

  /**
   * @brief Set the Conversion Mode
   *
   * @param mode  Conversion mode (CONTINUOUS, ONESHOT_STANDBY, ONESHOT_SHUTDOWN)
   * @return status_t
   *
   * @ingroup config
   * @see config3(), startContinuous(), stopContinuous()
   */
  status_t setConversionMode(conv_mode mode) {
    _config3.conv_mode = mode;
    return write(_config3);
  }

  /**
   * @brief Set the ADC operating mode
   *
   * @param mode  ADC mode (CONVERSION, STANDBY, SHUTDOWN)
   * @return status_t
   *
   * @ingroup config
   * @see config0()
   */
  status_t setAdcMode(adc_mode mode) {
    _config0.adc = mode;
    return write(_config0);
  }

  /**
   * @brief Set the Clock Selection
   *
   * @param clk  Clock source (INTERN_OUTPUT, INTERN, EXTERN)
   * @return status_t
   *
   * @ingroup config
   * @see config0()
   */
  status_t setClockSelection(clk_sel clk) {
    _config0.clk = clk;
    return write(_config0);
  }

  /**
   * @brief enable scanning on given channel
   *
   * @param chan  channel to add to scan list
   * @return status_t
   *
   * @ingroup config
   * @see scan(), disableScanChannel(), startScan()
   */
  status_t enableScanChannel(Mux chan) {
    for (size_t i = 0; i < sizeof(_channelID) / sizeof(_channelID[0]); i++) {
      if (_channelID[i] == chan.raw) {
        bitSet(_scan.channel.raw, i);
        break;
      }
    }
    return write(_scan);
  }

  /**
   * @brief disable scanning of given channel
   *
   * @param chan  channel to remove from scan list
   * @return status_t
   *
   * @ingroup config
   * @see scan(), enableScanChannel(), startScan()
   */
  status_t disableScanChannel(Mux chan) {
    for (size_t i = 0; i < sizeof(_channelID) / sizeof(_channelID[0]); i++) {
      if (_channelID[i] == chan.raw) {
        bitClear(_scan.channel.raw, i);
        break;
      }
    }
    return write(_scan);
  }

  /**
   * @brief set oversampling rate
   *
   * @param rate  Oversampling ratio (see osr enum)
   * @return status_t
   *
   * @ingroup calibration
   * @see config1(), setOversamplingRatio()
   */
  status_t setAveraging(osr rate) {
    _config1.osr = rate;
    return write(_config1);
  }

  status_t setOversamplingRatio(osr rate) { return setAveraging(rate); }

  void setResolution(size_t bits) {}  // not applicable for fixed-resolution external ADC

  void analogReadResolution(size_t bits) {}  // not applicable for fixed-resolution external ADC

  /**
   * @brief set reference voltage
   *
   * @param vref  reference voltage in volts (0.0 = use internal 2.4 V)
   * @return status_t
   *
   * @ingroup calibration
   * @see config0(), getReference()
   */
  status_t setReference(float vref = 0.0) {
    if (vref == 0.0) {
      vref              = 2.4;
      _config0.vref_sel = 1;
    } else {
      _config0.vref_sel = 0;
    }
    status_t ret = write(_config0);
    _reference   = vref;
    return ret;
  }

  /**
   * @brief get reference voltage
   *
   * @return float reference voltage in volts
   *
   * @ingroup status
   * @see setReference()
   */
  float getReference() { return _reference; }

  /**
   * @brief get SPI device address from last status byte
   *
   * The STATUS byte echoes the device address as STAT[5:4] = DEV_ADDR[1:0]
   * (STAT[3] duplicates STAT[4]). This method extracts DEV_ADDR[1:0] without
   * inversion per datasheet Section 6.2.6.
   *
   * @return uint8_t device address (0-3)
   *
   * @ingroup status
   */
  uint8_t getDeviceAddress() const { return (_status.addr >> 1) & 0x03; }

  /**
   * @brief convert raw ADC value to microvolts
   *
   * @param raw  raw sign-extended ADC value from readADCDATA() or analogRead()
   * @return int32_t voltage in microvolts (µV)
   *
   * @ingroup status
   * @see readADCDATA(), analogRead(), getReference(), getMaxValue()
   */
  int32_t toVoltage(int32_t raw) {
    int64_t VREF_uV = (int64_t)(_reference * 1000000.0f);
    int64_t num     = (int64_t)raw * VREF_uV;
    int64_t den     = (int64_t)(1UL << (_MAX_RESOLUTION - 1));

    if (_config2.gain == GAIN_033) {
      num *= 3;
    } else {
      den *= (int64_t)(1UL << (_config2.gain - 1));
    }

    return (int32_t)(num / den);
  }

  /**
   * @brief set analogRead timeout in milliseconds
   *
   * @param ms  timeout in ms (0 = disabled, blocking reads wait indefinitely)
   *
   * @ingroup init
   * @see getReadTimeout(), analogRead()
   */
  void setReadTimeout(uint32_t ms) { _readTimeout = ms; }

  /**
   * @brief get analogRead timeout in milliseconds
   *
   * @return uint32_t timeout in ms
   *
   * @ingroup init
   * @see setReadTimeout()
   */
  uint32_t getReadTimeout() { return _readTimeout; }

  /**
   * @brief read temperature sensor and return centi-Celsius
   *
   * @return int16_t temperature in 0.01°C (e.g. 2550 = 25.50°C)
   *
   * @ingroup calibration
   * @see analogRead(), toVoltage(), setTempOffset(), setTempSlope()
   */
  int16_t readTemperature() {
    uint32_t savedTimeout = _readTimeout;
    _readTimeout          = 0;
    int32_t raw           = analogRead(MCP3x6x_TEMP);
    _readTimeout          = savedTimeout;

    int32_t V_uV          = toVoltage(raw);

    int32_t V_25_uV;
    if (_MAX_RESOLUTION == 16) {
      V_25_uV = 684000;
    } else {
      V_25_uV = 690000;
    }

    return (int16_t)((int64_t)(V_uV - V_25_uV) * 100 / (int64_t)_tempSlope + 2500 + _tempOffset);
  }

  /**
   * @brief set temperature offset calibration (centi-Celsius)
   *
   * @param offset_cC  temperature offset in 0.01°C
   *
   * @ingroup calibration
   * @see readTemperature(), setTempSlope()
   */
  void setTempOffset(int16_t offset_cC) { _tempOffset = offset_cC; }

  /**
   * @brief set temperature slope calibration (µV/°C)
   *
   * @param slope_uV_per_C  temperature slope in µV per °C
   *
   * @ingroup calibration
   * @see readTemperature(), setTempOffset()
   */
  void setTempSlope(uint16_t slope_uV_per_C) { _tempSlope = slope_uV_per_C; }

  /**
   * @brief get maximal value for the ADC resolution
   *
   * @return uint32_t 2^(_MAX_RESOLUTION - 1), i.e. half-scale for signed data
   *
   * @ingroup status
   * @see toVoltage(), getReference()
   */
  uint32_t getMaxValue() { return (uint32_t)1 << (_MAX_RESOLUTION - 1); }

  /**
   * @brief conversion finished
   *
   * @return true  conversion complete and data ready
   * @return false  conversion in progress
   *
   * @ingroup status
   * @see status_dr(), dataAvailable()
   */
  bool isComplete() { return status_dr(); }

  /**
   * @brief starts a continuous conversion cycle
   *
   * Configures continuous conversion mode and triggers the first conversion.
   * Use analogRead() to read results without re-triggering.
   *
   * @ingroup conversion
   * @see stopContinuous(), setConversionMode(), conversion()
   */
  void startContinuous() {
    setConversionMode(conv_mode::CONTINUOUS);
    conversion();
  }

  /**
   * @brief stops a continuous conversion cycle
   *
   * Switches to one-shot standby mode and sends the standby command.
   *
   * @ingroup conversion
   * @see startContinuous(), setConversionMode(), standby()
   */
  void stopContinuous() {
    setConversionMode(conv_mode::ONESHOT_STANDBY);
    standby();
  }

  /**
   * @brief starts a continuous conversion cycle on differential channels
   *
   * Configures differential input mode and starts continuous conversion.
   *
   * @ingroup conversion
   * @see differentialMode(), startContinuous()
   */
  void startContinuousDifferential() {
    differentialMode();
    startContinuous();
  }

  /**
   * @brief checks if continuous mode is set
   *
   * @return true  continuous conversion mode active
   * @return false  one-shot mode active
   *
   * @ingroup status
   * @see startContinuous(), stopContinuous()
   */
  bool isContinuous() { return _config3.conv_mode == conv_mode::CONTINUOUS; }

  /**
   * @brief start scan mode: configure IRQ pin and set ADC to Conversion
   *
   * Enables continuous conversion with IRQ-driven data ready signalling.
   * Call IRQ_handler() from your ISR to process conversions.
   *
   * @param mode   IRQ pin mode (IRQ_OPEN_DRAIN, IRQ_PUSH_PULL)
   * @param fmt    ADC output data format
   * @return status_t
   *
   * @ingroup conversion
   * @see startScanOnce(), stopContinuous(), IRQ_handler(), attachIRQ()
   */
  status_t startScan(enum irq_mode mode = IRQ_OPEN_DRAIN,
                     data_format fmt    = data_format::ID_SGNEXT_DATA) {
    _irq.en_fastcmd      = 1;
    _irq.irq_mode        = (uint8_t)mode;
    status_t ret         = write(_irq);
    _config3.data_format = fmt;
    _config3.conv_mode   = conv_mode::CONTINUOUS;
    ret                  = write(_config3);
    _config0.adc         = adc_mode::CONVERSION;
    ret                  = write(_config0);
    return conversion();
  }

  /**
   * @brief start a single scan cycle (one-shot) and standby afterwards
   *
   * Performs one scan across enabled channels, then enters standby mode.
   *
   * @return status_t
   *
   * @ingroup conversion
   * @see startScan(), stopContinuous(), standby()
   */
  status_t startScanOnce() {
    _irq.en_fastcmd      = 1;
    _irq.irq_mode        = (uint8_t)IRQ_OPEN_DRAIN;
    status_t ret         = write(_irq);
    _config3.data_format = data_format::ID_SGNEXT_DATA;
    _config3.conv_mode   = conv_mode::ONESHOT_STANDBY;
    ret                  = write(_config3);
    _config0.adc         = adc_mode::CONVERSION;
    ret                  = write(_config0);
    return conversion();
  }

  /**
   * @brief set single ended mode
   *
   * Configures the MUX for single-ended inputs (VIN- = AGND).
   *
   * @return status_t
   *
   * @ingroup conversion
   * @see differentialMode(), mux()
   */
  status_t singleEndedMode() {
    // Configure for single-ended mode by setting VIN- to AGND
    _mux.vin_minus = MUX_AGND;
    return write(_mux);
  }

  /**
   * @brief set differential mode
   *
   * Configures the MUX for differential inputs (default CH0-CH1 pair).
   *
   * @return status_t
   *
   * @ingroup conversion
   * @see singleEndedMode(), mux(), startContinuousDifferential()
   */
  status_t differentialMode() {
    // Configure for differential mode by setting VIN- to a channel
    // Default to CH0-CH1 differential pair
    _mux.vin_plus  = MUX_CH0;
    _mux.vin_minus = MUX_CH1;
    return write(_mux);
  }

  /**
   * @brief returns mode
   *
   * @return true  differential mode is set (different plus/minus channels)
   * @return false  single-ended mode is set
   *
   * @ingroup status
   * @see differentialMode(), singleEndedMode()
   */
  bool isDifferential() {
    return (_mux.vin_plus < 8 && _mux.vin_minus < 8 && _mux.vin_plus != _mux.vin_minus);
  }

  /**
   * @brief returns analog value of given channel
   *
   * In mux mode (scan not active): configures the MUX, triggers a conversion,
   * and blocks until data is ready (respecting the read timeout).
   * In scan mode: returns the cached result for the channel.
   *
   * @param chan  channel to read (MCP3x6x_CH0 … MCP3x6x_CH7, or internal channels)
   * @return int32_t sign-extended ADC value, or MCP3x6x_TIMEOUT on timeout
   *
   * @ingroup conversion
   * @see readADCDATA(), toVoltage(), setReadTimeout(), startScan()
   */
  int32_t analogRead(Mux chan = MCP3x6x_CH0) {
    // MuxMode
    if (_scan.channel.raw == 0) {
      _mux = chan;
      write(_mux);
      conversion();

      uint32_t start = millis();
      do {
        read(&_adcdata);
        if (_readTimeout && (millis() - start) >= _readTimeout) {
          return MCP3x6x_TIMEOUT;
        }
      } while (_status.dr);

      _dataAvailable                                  = false;
      return _result.raw[(uint8_t)_adcdata.channelid] = _adcdata.value;
    }

    // ScanMode — return cached result unless DMA backs the scan
    if (_dmaEnabled) {
      int16_t idx = -1;
      for (size_t i = 0; i < sizeof(_channelID) / sizeof(_channelID[0]); i++) {
        if (_channelID[i] == chan.raw) {
          idx = (int16_t)i;
          break;
        }
      }
      if (idx < 0) {
        return -1;
      }
      uint32_t start = millis();
      for (;;) {
        _dmaPollEngine();
        DmaSample s;
        // drain FIFO in order: consume other channels until requested appears
        if (_dmaRing.pop(s)) {
          if (s.channel == (uint8_t)idx) {
            return s.value;
          }
          continue;
        }
        if (_readTimeout && (millis() - start) >= _readTimeout) {
          return MCP3x6x_TIMEOUT;
        }
      }
    }
    for (size_t i = 0; i < sizeof(_channelID) / sizeof(_channelID[0]); i++) {
      if (_channelID[i] == chan.raw) {
        _dataAvailable = false;
        return _result.raw[i];
      }
    }
    return -1;
  }

  /**
   * @brief read cached result for a channel in continuous mode
   *
   * @param chan  channel to read
   * @return int32_t cached ADC value, or -1 if not in continuous mode / channel not found
   *
   * @ingroup conversion
   * @see startContinuous(), isContinuous(), analogRead()
   */
  int32_t analogReadContinuous(Mux chan) {
    if (isContinuous()) {
      for (size_t i = 0; i < sizeof(_channelID) / sizeof(_channelID[0]); i++) {
        if (_channelID[i] == chan.raw) {
          return _result.raw[i];
        }
      }
    }
    return -1;
  }

  /**
   * @brief read differential voltage between two channels
   *
   * Configures the MUX for the pinP/pinN differential pair, triggers a
   * conversion, and blocks until data is ready.
   *
   * @param pinP  positive input channel selector
   * @param pinN  negative input channel selector
   * @return int32_t sign-extended differential ADC value, or MCP3x6x_TIMEOUT on timeout
   *
   * @ingroup conversion
   * @see differentialMode(), analogRead(), toVoltage()
   */
  int32_t analogReadDifferential(Mux pinP, Mux pinN) {
    // Map differential inputs: each side's VIN+ selects its channel,
    // the opposite side selects the negative input. Any 4-bit selector is allowed.
    _mux.vin_plus  = pinP.vin_plus;
    _mux.vin_minus = pinN.vin_plus;
    write(_mux);

    conversion();
    uint32_t start = millis();
    do {
      read(&_adcdata);
      if (_readTimeout && (millis() - start) >= _readTimeout) {
        return MCP3x6x_TIMEOUT;
      }
    } while (_status.dr);
    return _result.raw[(uint8_t)_adcdata.channelid] = _adcdata.value;
  }
};

// type aliases for concrete devices:
using MCP3461 = MCP3x6x<16, 2>;
using MCP3462 = MCP3x6x<16, 4>;
using MCP3464 = MCP3x6x<16, 8>;
using MCP3561 = MCP3x6x<24, 2>;
using MCP3562 = MCP3x6x<24, 4>;
using MCP3564 = MCP3x6x<24, 8>;

using MCP3465 = MCP3x6x<16, 2>;  // 2-channel UQFN variant
using MCP3565 = MCP3x6x<24, 2>;  // 2-channel UQFN variant

#endif  // SRC_ARDUINO_MCP3X6X_LIBRARY_HPP_
