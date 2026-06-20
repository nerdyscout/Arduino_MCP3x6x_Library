// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#include "__init__.h"

#ifdef PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// All 5 fast commands send correct bytes
// ---------------------------------------------------------------------------
void test_conversion_fastcmd() {
  TestADC adc; setUp();
  adc.conversion();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_CONVERSION, spiAddrs[0]);
}

void test_standby_fastcmd() {
  TestADC adc; setUp();
  adc.standby();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_STANDBY, spiAddrs[0]);
}

void test_shutdown_fastcmd() {
  TestADC adc; setUp();
  adc.shutdown();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_SHUTDOWN, spiAddrs[0]);
}

void test_full_shutdown_fastcmd() {
  TestADC adc; setUp();
  adc.full_shutdown();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_FULL_SHUTDOWN, spiAddrs[0]);
}

void test_reset_fastcmd() {
  TestADC adc; setUp();
  adc.reset();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_RESET, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// Status flag getters
// ---------------------------------------------------------------------------
void test_status_dr_true() {
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1 → _status.dr=1 → status_dr() = !1 = false
  adc.reset();
  TEST_ASSERT_FALSE(adc.status_dr());
}

void test_status_dr_false() {
  TestADC adc;
  mockStatusByte = 0x00;  // DR=0 → _status.dr=0 → status_dr() = !0 = true
  adc.reset();
  TEST_ASSERT_TRUE(adc.status_dr());
}

void test_status_crccfg() {
  TestADC adc;
  mockStatusByte = 0x08;  // CRCCFG=1 → inverted → false
  adc.reset();
  TEST_ASSERT_FALSE(adc.status_crccfg());
}

void test_status_por() {
  TestADC adc;
  mockStatusByte = 0x10;  // POR=1 → inverted → false
  adc.reset();
  TEST_ASSERT_FALSE(adc.status_por());
}

void test_status_raw() {
  TestADC adc;
  mockStatusByte = 0xAB;
  adc.reset();
  TEST_ASSERT_EQUAL(0xAB, adc.status());
}

// ---------------------------------------------------------------------------
// isComplete() is equivalent to status_dr()
// ---------------------------------------------------------------------------
void test_isComplete() {
  TestADC adc;
  mockStatusByte = 0x04;
  adc.reset();
  TEST_ASSERT_EQUAL(adc.status_dr(), adc.isComplete());
}

// ---------------------------------------------------------------------------
// available() returns status_dr()
// ---------------------------------------------------------------------------
void test_available() {
  TestADC adc;
  mockStatusByte = 0x04;
  adc.reset();
  TEST_ASSERT_EQUAL(adc.status_dr(), adc.available());
}

// ---------------------------------------------------------------------------
// _getValue — sign extension via arithmetic shift (16-bit SGN_DATA)
// ---------------------------------------------------------------------------
void test_getValue_16bit_SGN_DATA() {
  TestADC adc;
  adc.setDataFormat(TestADC::SGN_DATA);
  int32_t v = MCP3x6xTest::getValue(adc, 0x0000FFFF);
  TEST_ASSERT_EQUAL(-1, v);
}

// ---------------------------------------------------------------------------
// _getValue — 24-bit SGN_DATA sign extension
// ---------------------------------------------------------------------------
void test_getValue_24bit_SGN_DATA() {
  MCP3564 adc;
  adc.setDataFormat(MCP3564::SGN_DATA);
  int32_t v = MCP3x6xTest::getValue(adc, 0x00FFFFFF);
  TEST_ASSERT_EQUAL(-1, v);
}

// ---------------------------------------------------------------------------
// _getValue — SGN_DATA_ZERO (24-bit left justified)
// ---------------------------------------------------------------------------
void test_getValue_16bit_SGN_DATA_ZERO() {
  TestADC adc;
  adc.setDataFormat(TestADC::SGN_DATA_ZERO);
  int32_t v = MCP3x6xTest::getValue(adc, 0x00FF0000);
  TEST_ASSERT_EQUAL(0xFF, v);  // raw >> 16
}

void test_getValue_24bit_SGN_DATA_ZERO() {
  MCP3564 adc;
  adc.setDataFormat(MCP3564::SGN_DATA_ZERO);
  int32_t v = MCP3x6xTest::getValue(adc, 0x00FFFF00);
  TEST_ASSERT_EQUAL(0xFFFF, v);  // raw >> 8
}

// ---------------------------------------------------------------------------
// _getValue — SGNEXT_DATA / ID_SGNEXT_DATA returns raw unchanged
// ---------------------------------------------------------------------------
void test_getValue_SGNEXT_16bit() {
  TestADC adc;
  adc.setDataFormat(TestADC::SGNEXT_DATA);
  int32_t v = MCP3x6xTest::getValue(adc, 0x7FFFFFFF);
  TEST_ASSERT_EQUAL(0x7FFFFFFF, v);
}

void test_getValue_ID_SGNEXT_24bit() {
  MCP3564 adc;
  adc.setDataFormat(MCP3564::ID_SGNEXT_DATA);
  int32_t v = MCP3x6xTest::getValue(adc, 0x12345678);
  TEST_ASSERT_EQUAL(0x12345678, v);
}

// ---------------------------------------------------------------------------
// setDataFormat() writes CONFIG3
// ---------------------------------------------------------------------------
void test_setDataFormat_writes_config3() {
  TestADC adc; adc.begin(); setUp();
  adc.setDataFormat(TestADC::SGNEXT_DATA);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
}

#endif  // PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_fastcmd_status_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_conversion_fastcmd);
  RUN_TEST(test_standby_fastcmd);
  RUN_TEST(test_shutdown_fastcmd);
  RUN_TEST(test_full_shutdown_fastcmd);
  RUN_TEST(test_reset_fastcmd);
  RUN_TEST(test_status_dr_true);
  RUN_TEST(test_status_dr_false);
  RUN_TEST(test_status_crccfg);
  RUN_TEST(test_status_por);
  RUN_TEST(test_status_raw);
  RUN_TEST(test_isComplete);
  RUN_TEST(test_available);
  RUN_TEST(test_getValue_16bit_SGN_DATA);
  RUN_TEST(test_getValue_24bit_SGN_DATA);
  RUN_TEST(test_getValue_16bit_SGN_DATA_ZERO);
  RUN_TEST(test_getValue_24bit_SGN_DATA_ZERO);
  RUN_TEST(test_getValue_SGNEXT_16bit);
  RUN_TEST(test_getValue_ID_SGNEXT_24bit);
  RUN_TEST(test_setDataFormat_writes_config3);
#endif
}
