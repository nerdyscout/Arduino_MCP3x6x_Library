// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// CMD_* macro values
// ---------------------------------------------------------------------------
void test_cmd_macro_values() {
#ifdef PIO_NATIVE_TESTING
  TEST_ASSERT_EQUAL_HEX8(0x68, MCP3x6x_CMD_CONVERSION);
  TEST_ASSERT_EQUAL_HEX8(0x6C, MCP3x6x_CMD_STANDBY);
  TEST_ASSERT_EQUAL_HEX8(0x70, MCP3x6x_CMD_SHUTDOWN);
  TEST_ASSERT_EQUAL_HEX8(0x74, MCP3x6x_CMD_FULL_SHUTDOWN);
  TEST_ASSERT_EQUAL_HEX8(0x78, MCP3x6x_CMD_RESET);
  TEST_ASSERT_EQUAL_HEX8(0x41, MCP3x6x_CMD_SREAD);
  TEST_ASSERT_EQUAL_HEX8(0x43, MCP3x6x_CMD_IREAD);
  TEST_ASSERT_EQUAL_HEX8(0x42, MCP3x6x_CMD_IWRITE);
#else
  // Macro values verified by native.
#endif
}

// ---------------------------------------------------------------------------
// ADR_* macro values
// ---------------------------------------------------------------------------
void test_adr_macro_values() {
#ifdef PIO_NATIVE_TESTING
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x0 << 2), MCP3x6x_ADR_ADCDATA);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x1 << 2), MCP3x6x_ADR_CONFIG0);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x2 << 2), MCP3x6x_ADR_CONFIG1);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x3 << 2), MCP3x6x_ADR_CONFIG2);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x4 << 2), MCP3x6x_ADR_CONFIG3);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x5 << 2), MCP3x6x_ADR_IRQ);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x6 << 2), MCP3x6x_ADR_MUX);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x7 << 2), MCP3x6x_ADR_SCAN);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x8 << 2), MCP3x6x_ADR_TIMER);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0x9 << 2), MCP3x6x_ADR_OFFSET);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0xA << 2), MCP3x6x_ADR_GAIN);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0xD << 2), MCP3x6x_ADR_LOCK);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_SPI_ADR | (0xF << 2), MCP3x6x_ADR_CRCCFG);
#else
  // Macro values verified by native.
#endif
}

// ---------------------------------------------------------------------------
// CFG_* default values
// ---------------------------------------------------------------------------
void test_cfg_default_values() {
#ifdef PIO_NATIVE_TESTING
  TEST_ASSERT_EQUAL(0xC0, MCP3x6x_CFG_CONFIG0);
  TEST_ASSERT_EQUAL(0x0C, MCP3x6x_CFG_CONFIG1);
  TEST_ASSERT_EQUAL(0x8B, MCP3x6x_CFG_CONFIG2);
  TEST_ASSERT_EQUAL(0x00, MCP3x6x_CFG_CONFIG3);
  TEST_ASSERT_EQUAL(0x73, MCP3x6x_CFG_IRQ);
  TEST_ASSERT_EQUAL(0x08, MCP3x6x_CFG_MUX);
  TEST_ASSERT_EQUAL(0xA5, MCP3x6x_CFG_LOCK);
#else
  // Config defaults verified by native.
#endif
}

// ---------------------------------------------------------------------------
// Channel macro values
// ---------------------------------------------------------------------------
void test_channel_macro_values() {
#ifdef PIO_NATIVE_TESTING
  TEST_ASSERT_EQUAL_HEX8(0x08, MCP3x6x_CH0);
  TEST_ASSERT_EQUAL_HEX8(0x18, MCP3x6x_CH1);
  TEST_ASSERT_EQUAL_HEX8(0x28, MCP3x6x_CH2);
  TEST_ASSERT_EQUAL_HEX8(0x38, MCP3x6x_CH3);
  TEST_ASSERT_EQUAL_HEX8(0x48, MCP3x6x_CH4);
  TEST_ASSERT_EQUAL_HEX8(0x58, MCP3x6x_CH5);
  TEST_ASSERT_EQUAL_HEX8(0x68, MCP3x6x_CH6);
  TEST_ASSERT_EQUAL_HEX8(0x78, MCP3x6x_CH7);
  TEST_ASSERT_EQUAL_HEX8(0x01, MCP3x6x_DIFFA);
  TEST_ASSERT_EQUAL_HEX8(0x23, MCP3x6x_DIFFB);
  TEST_ASSERT_EQUAL_HEX8(0x45, MCP3x6x_DIFFC);
  TEST_ASSERT_EQUAL_HEX8(0x67, MCP3x6x_DIFFD);
  TEST_ASSERT_EQUAL_HEX8(0xDE, MCP3x6x_TEMP);
  TEST_ASSERT_EQUAL_HEX8(0x98, MCP3x6x_AVDD);
  TEST_ASSERT_EQUAL_HEX8(0xF8, MCP3x6x_VCM);
  TEST_ASSERT_EQUAL_HEX8(0x88, MCP3x6x_OFFSET);
#else
  // Channel macros verified by native.
#endif
}

// ---------------------------------------------------------------------------
// Enum values match datasheet bit patterns
// ---------------------------------------------------------------------------
void test_enum_values_all() {
#ifdef PIO_NATIVE_TESTING
  // adc_mode
  TEST_ASSERT_EQUAL(3, (uint8_t)TestADC::CONVERSION);
  TEST_ASSERT_EQUAL(2, (uint8_t)TestADC::STANDBY);
  TEST_ASSERT_EQUAL(0, (uint8_t)TestADC::SHUTDOWN);
  // clk_sel
  TEST_ASSERT_EQUAL(3, (uint8_t)TestADC::INTERN_OUTPUT);
  TEST_ASSERT_EQUAL(2, (uint8_t)TestADC::INTERN);
  TEST_ASSERT_EQUAL(0, (uint8_t)TestADC::EXTERN);
  // osr
  TEST_ASSERT_EQUAL(15, (uint8_t)TestADC::OSR_98304);
  TEST_ASSERT_EQUAL(0, (uint8_t)TestADC::OSR_32);
  // gain
  TEST_ASSERT_EQUAL(7, (uint8_t)TestADC::GAIN_64);
  TEST_ASSERT_EQUAL(1, (uint8_t)TestADC::GAIN_1);
  TEST_ASSERT_EQUAL(0, (uint8_t)TestADC::GAIN_033);
  // data_format
  TEST_ASSERT_EQUAL(3, (uint8_t)TestADC::ID_SGNEXT_DATA);
  TEST_ASSERT_EQUAL(0, (uint8_t)TestADC::SGN_DATA);
  // conv_mode
  TEST_ASSERT_EQUAL(3, (uint8_t)TestADC::CONTINUOUS);
  TEST_ASSERT_EQUAL(2, (uint8_t)TestADC::ONESHOT_STANDBY);
  TEST_ASSERT_EQUAL(0, (uint8_t)TestADC::ONESHOT_SHUTDOWN);
  // mux
  TEST_ASSERT_EQUAL(0, (uint8_t)TestADC::MUX_CH0);
  TEST_ASSERT_EQUAL(15, (uint8_t)TestADC::MUX_VCM);
#else
  // Enum values verified by native.
#endif
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_spi_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_cmd_macro_values);
  RUN_TEST(test_adr_macro_values);
  RUN_TEST(test_cfg_default_values);
  RUN_TEST(test_channel_macro_values);
  RUN_TEST(test_enum_values_all);
#endif
}
