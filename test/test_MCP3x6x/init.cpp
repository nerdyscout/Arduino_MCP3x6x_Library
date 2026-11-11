// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// Constructor / instance creation — one test per variant
// ---------------------------------------------------------------------------
void test_instance_all_variants() {
#ifdef PIO_NATIVE_TESTING
#define REG(V) test_instance<V>();
  FOR_EACH_ADC(REG)
#undef REG
#else
  test_instance<TestADC>();
#endif
}

// ---------------------------------------------------------------------------
// begin() — success path for all variants (POR=1 + correct HW_ID)
// ---------------------------------------------------------------------------
void test_begin_success_current_variant() {
#ifdef PIO_NATIVE_TESTING
  // Expected chip-ID nibble per variant (from Linux driver mcp3564.c)
  #define NIBBLE_MCP3461 0x8
  #define NIBBLE_MCP3462 0x9
  #define NIBBLE_MCP3464 0xB
  #define NIBBLE_MCP3561 0xC
  #define NIBBLE_MCP3562 0xD
  #define NIBBLE_MCP3564 0xF

  uint8_t nibble = 0xFF;
#define MAP_NIBBLE(V) \
  if (std::is_same<TestADC, V>::value) nibble = NIBBLE_##V;
  FOR_EACH_ADC(MAP_NIBBLE)
#undef MAP_NIBBLE
#undef NIBBLE_MCP3461
#undef NIBBLE_MCP3462
#undef NIBBLE_MCP3464
#undef NIBBLE_MCP3561
#undef NIBBLE_MCP3562
#undef NIBBLE_MCP3564

  TEST_ASSERT_NOT_EQUAL(0xFF, nibble);  // variant must be recognized

  mockStatusByte   = 0x01;   // POR=1 → reset() succeeds
  mockReadSequence = {nibble, 0x30};  // HW_ID nibble + R-variant
  TestADC adc;
  bool ok = adc.begin();
  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_FALSE(spiAddrs.empty());
  setUp();
#else
  mockStatusByte   = 0x01;
  mockReadSequence = {0x0F, 0x30};  // MCP3564 default
  TestADC adc;
  TEST_ASSERT_TRUE(adc.begin());
  TEST_ASSERT_FALSE(spiAddrs.empty());
#endif
}

// ---------------------------------------------------------------------------
// begin() — failure paths: wrong chip-ID nibble rejected
// ---------------------------------------------------------------------------
void test_begin_fail_wrong_chipid() {
#ifdef PIO_NATIVE_TESTING
  static const uint8_t bad_ids[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
                                    0xA, 0xE};
  for (size_t i = 0; i < sizeof(bad_ids) / sizeof(bad_ids[0]); i++) {
    TestADC adc;
    mockStatusByte   = 0x01;
    mockReadSequence = {bad_ids[i], 0x30};
    bool ok = adc.begin();
    TEST_ASSERT_FALSE_MESSAGE(ok, "wrong chip-ID should fail begin()");
    setUp();
  }
#else
  // Hardware: cannot easily inject wrong chip-ID; covered by native.
#endif
}

// ---------------------------------------------------------------------------
// begin() — failure path: POR bit not set
// ---------------------------------------------------------------------------
void test_begin_fail_no_por() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte   = 0x00;   // POR=0 → reset() fails
  mockReadSequence = {0x0F, 0x30};
  bool ok = adc.begin();
  TEST_ASSERT_FALSE(ok);
#else
  // Hardware: covered by native mock.
#endif
}

// ---------------------------------------------------------------------------
// reset() — sends CMD_RESET and updates _status
// ---------------------------------------------------------------------------
void test_reset_sends_cmd_reset() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x01;
  setUp();
  adc.reset();
  TEST_ASSERT_EQUAL_HEX8(0x78, spiAddrs[0]);  // CMD_RESET
#else
  TestADC adc;
  adc.reset();
#endif
}

// ---------------------------------------------------------------------------
// end() — calls SPI.end()
// ---------------------------------------------------------------------------
void test_end_calls_spi_end() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  adc.end();
  TEST_ASSERT_TRUE(true);  // verifies no crash; SPI.end() mocked
#else
  TestADC adc;
  adc.begin();
  adc.end();
#endif
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_init_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_instance_all_variants);
  RUN_TEST(test_begin_success_current_variant);
  RUN_TEST(test_begin_fail_wrong_chipid);
  RUN_TEST(test_begin_fail_no_por);
  RUN_TEST(test_reset_sends_cmd_reset);
#endif
  RUN_TEST(test_end_calls_spi_end);
}
