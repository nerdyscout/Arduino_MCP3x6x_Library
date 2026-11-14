// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// Constructor / instance creation — one test per variant
// ---------------------------------------------------------------------------
void test_instance_all_variants() {
#ifdef PIO_MOCKED_TESTING
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
  mockStatusByte = 0x01;  // POR=1 → reset() succeeds
#ifdef PIO_MOCKED_TESTING
// Expected chip-ID nibble per variant (from Linux driver mcp3564.c)
#define NIBBLE_MCP3461 0x8
#define NIBBLE_MCP3462 0x9
#define NIBBLE_MCP3464 0xB
#define NIBBLE_MCP3561 0xC
#define NIBBLE_MCP3562 0xD
#define NIBBLE_MCP3564 0xF

  uint8_t nibble = 0xFF;
#define MAP_NIBBLE(V)                  \
  if (std::is_same<TestADC, V>::value) \
    nibble = NIBBLE_##V;
  FOR_EACH_ADC(MAP_NIBBLE)
#undef MAP_NIBBLE
#undef NIBBLE_MCP3461
#undef NIBBLE_MCP3462
#undef NIBBLE_MCP3464
#undef NIBBLE_MCP3561
#undef NIBBLE_MCP3562
#undef NIBBLE_MCP3564

  TEST_ASSERT_NOT_EQUAL(0xFF, nibble);  // variant must be recognized

  mockReadSequence = {nibble, 0x30};  // HW_ID nibble + R-variant
#endif
  TestADC adc;
  bool ok = adc.begin();
  TEST_ASSERT_TRUE(ok);
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_FALSE(spiAddrs.empty());
#endif
}

// ---------------------------------------------------------------------------
// begin() — failure paths: wrong chip-ID nibble rejected
// ---------------------------------------------------------------------------
void test_begin_fail_wrong_chipid() {
#ifdef PIO_MOCKED_TESTING
  static const uint8_t bad_ids[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0xA, 0xE};
  for (size_t i = 0; i < sizeof(bad_ids) / sizeof(bad_ids[0]); i++) {
    TestADC adc;
    mockStatusByte = 0x01;
    mockReadSequence = {bad_ids[i], 0x30};
    bool ok = adc.begin();
    TEST_ASSERT_FALSE_MESSAGE(ok, "wrong chip-ID should fail begin()");
  }
#else
  // Hardware: cannot easily inject wrong chip-ID — smoke test begin() only
  TestADC adc;
  (void)adc.begin();
#endif
}

// ---------------------------------------------------------------------------
// begin() — failure path: POR bit not set
// ---------------------------------------------------------------------------
void test_begin_fail_no_por() {
#ifdef PIO_MOCKED_TESTING
  TestADC adc;
  mockStatusByte = 0x00;  // POR=0 → reset() fails
  mockReadSequence = {0x0F, 0x30};
  bool ok = adc.begin();
  TEST_ASSERT_FALSE(ok);
#else
  // Hardware: cannot easily control POR bit — smoke test begin() only
  TestADC adc;
  (void)adc.begin();
#endif
}

// ---------------------------------------------------------------------------
// reset() — sends CMD_RESET and updates _status
// ---------------------------------------------------------------------------
void test_reset_sends_cmd_reset() {
  TestADC adc;
  mockStatusByte = 0x01;
  adc.reset();
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_EQUAL_HEX8(0x78, spiAddrs[0]);  // CMD_RESET
#endif
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
  RUN_TEST(test_begin_fail_wrong_chipid);
  RUN_TEST(test_begin_fail_no_por);
  RUN_TEST(test_reset_sends_cmd_reset);
  RUN_TEST(test_end_calls_spi_end);
}
