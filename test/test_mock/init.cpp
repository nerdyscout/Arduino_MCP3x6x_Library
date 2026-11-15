// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// Constructor / instance creation — one test per variant
// ---------------------------------------------------------------------------
void test_instance_all_variants() {
#define REG(V) test_instance<V>();
  FOR_EACH_ADC(REG)
#undef REG
}

// ---------------------------------------------------------------------------
// begin() — success path (reset issued; identity precomputed from template)
// ---------------------------------------------------------------------------
void test_begin_success_current_variant() {
  mockStatusByte = 0x01;
  TestADC adc;
  bool ok = adc.begin();
  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_FALSE(spiAddrs.empty());
}

// ---------------------------------------------------------------------------
// Chip ID is precomputed from <resolution, channels>
// ---------------------------------------------------------------------------
void test_chipid_precomputed_all_variants() {
  TEST_ASSERT_EQUAL_HEX8(0x8, MCP3461::kChipId);
  TEST_ASSERT_EQUAL_HEX8(0x9, MCP3462::kChipId);
  TEST_ASSERT_EQUAL_HEX8(0xB, MCP3464::kChipId);
  TEST_ASSERT_EQUAL_HEX8(0xC, MCP3561::kChipId);
  TEST_ASSERT_EQUAL_HEX8(0xD, MCP3562::kChipId);
  TEST_ASSERT_EQUAL_HEX8(0xF, MCP3564::kChipId);
}

// ---------------------------------------------------------------------------
// begin() does not require POR bit in reset response
// ---------------------------------------------------------------------------
void test_begin_succeeds_without_por_bit() {
  TestADC adc;
  mockStatusByte = 0x00;  // POR not set in status byte
  bool ok = adc.begin();
  TEST_ASSERT_TRUE(ok);
}

// ---------------------------------------------------------------------------
// reset() — sends CMD_RESET and updates _status
// ---------------------------------------------------------------------------
void test_reset_sends_cmd_reset() {
  TestADC adc;
  mockStatusByte = 0x01;
  adc.reset();
  TEST_ASSERT_EQUAL_HEX8(0x78, spiAddrs[0]);  // CMD_RESET
}

// ---------------------------------------------------------------------------
// end() — calls SPI.end()
// ---------------------------------------------------------------------------
void test_end_calls_spi_end() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.end();
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_init_tests() {
  RUN_TEST(test_instance_all_variants);
  RUN_TEST(test_begin_success_current_variant);
  RUN_TEST(test_chipid_precomputed_all_variants);
  RUN_TEST(test_begin_succeeds_without_por_bit);
  RUN_TEST(test_reset_sends_cmd_reset);
  RUN_TEST(test_end_calls_spi_end);
}
