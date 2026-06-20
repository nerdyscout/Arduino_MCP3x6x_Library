// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#include "__init__.h"

// ---------------------------------------------------------------------------
// Instance creation
// ---------------------------------------------------------------------------
void test_instance_MCP3461() { test_instance<MCP3461>(); }
void test_instance_MCP3462() { test_instance<MCP3462>(); }
void test_instance_MCP3464() { test_instance<MCP3464>(); }
void test_instance_MCP3561() { test_instance<MCP3561>(); }
void test_instance_MCP3562() { test_instance<MCP3562>(); }
void test_instance_MCP3564() { test_instance<MCP3564>(); }

// ---------------------------------------------------------------------------
// begin() — requires SPI mock + status byte with POR bit set
// ---------------------------------------------------------------------------
#ifdef PIO_NATIVE_TESTING
void test_begin_MCP3461() {
  mockStatusByte = 0x10;  // POR=1 (bit 4 set) → begin() returns true
  test_begin<MCP3461>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3462() {
  mockStatusByte = 0x10;
  test_begin<MCP3462>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3464() {
  mockStatusByte = 0x10;
  test_begin<MCP3464>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3561() {
  mockStatusByte = 0x10;
  test_begin<MCP3561>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3562() {
  mockStatusByte = 0x10;
  test_begin<MCP3562>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3564() {
  mockStatusByte = 0x10;
  test_begin<MCP3564>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
#endif

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_basic_tests() {
  #define REG_INSTANCE(V) RUN_TEST(test_instance_##V);
  FOR_EACH_ADC(REG_INSTANCE)
  #undef REG_INSTANCE

  #ifdef PIO_NATIVE_TESTING
    #define REG_BEGIN(V) RUN_TEST(test_begin_##V);
    FOR_EACH_ADC(REG_BEGIN)
    #undef REG_BEGIN
  #endif
}
