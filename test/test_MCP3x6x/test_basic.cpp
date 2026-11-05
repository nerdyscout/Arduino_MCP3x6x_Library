// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#include "test.h"

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
  mockStatusByte   = 0x01;  // POR=1 → reset() succeeds
  mockReadSequence  = {0x08, 0x30};  // HW_ID=MCP3461, R-variant
  test_begin<MCP3461>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3462() {
  mockStatusByte   = 0x01;
  mockReadSequence  = {0x09, 0x30};  // HW_ID=MCP3462, R-variant
  test_begin<MCP3462>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3464() {
  mockStatusByte   = 0x01;
  mockReadSequence  = {0x0B, 0x30};  // HW_ID=MCP3464, R-variant
  test_begin<MCP3464>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3561() {
  mockStatusByte   = 0x01;
  mockReadSequence  = {0x0C, 0x30};  // HW_ID=MCP3561, R-variant
  test_begin<MCP3561>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3562() {
  mockStatusByte   = 0x01;
  mockReadSequence  = {0x0D, 0x30};  // HW_ID=MCP3562, R-variant
  test_begin<MCP3562>();
  TEST_ASSERT_FALSE(spiAddrs.empty());
}
void test_begin_MCP3564() {
  mockStatusByte   = 0x01;
  mockReadSequence  = {0x0F, 0x30};  // HW_ID=MCP3564, R-variant
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
