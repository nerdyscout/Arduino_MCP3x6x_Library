// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// status_* getters (inverted logic: status_dr() = true when DR=1)
// ---------------------------------------------------------------------------
void test_status_dr_true() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1 → status_dr() = true
  adc.reset();
  TEST_ASSERT_TRUE(adc.status_dr());
#else
  TestADC adc;
  adc.begin();
  adc.status_dr();
#endif
}

void test_status_dr_false() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x00;  // DR=0 → status_dr() = false
  adc.reset();
  TEST_ASSERT_FALSE(adc.status_dr());
#else
  TestADC adc;
  adc.begin();
  adc.status_dr();
#endif
}

void test_status_crccfg() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x02;  // CRCCFG=1 (bit 1) → inverted → false
  adc.reset();
  TEST_ASSERT_FALSE(adc.status_crccfg());
#else
  TestADC adc;
  adc.begin();
  adc.status_crccfg();
#endif
}

void test_status_por() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x01;  // POR=1 (bit 0) → inverted → false
  adc.reset();
  TEST_ASSERT_FALSE(adc.status_por());
#else
  TestADC adc;
  adc.begin();
  adc.status_por();
#endif
}

void test_status_raw() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0xAB;
  adc.reset();
  TEST_ASSERT_EQUAL_HEX8(0xAB, adc.status());
#else
  TestADC adc;
  adc.begin();
  adc.status();
#endif
}

// ---------------------------------------------------------------------------
// isComplete delegates to status_dr
// ---------------------------------------------------------------------------
void test_is_complete_equals_status_dr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x04;
  adc.reset();
  TEST_ASSERT_EQUAL(adc.status_dr(), adc.isComplete());
#else
  TestADC adc;
  adc.begin();
  adc.isComplete();
#endif
}

// ---------------------------------------------------------------------------
// getDeviceAddress
// ---------------------------------------------------------------------------
void test_get_device_address_extracts_nibble() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x30;  // bits [5:3] = 110 → DEV_ADDR[1:0] = 3
  adc.begin();
  TEST_ASSERT_EQUAL(3, adc.getDeviceAddress());
#else
  TestADC adc;
  adc.begin();
  adc.getDeviceAddress();
#endif
}

// ---------------------------------------------------------------------------
// toVoltage — datasheet conversion V_uV = raw * VREF / 2^(N-1) / gain
// ---------------------------------------------------------------------------
void test_to_voltage_gain1_full_scale() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  adc.setReference(2.4f);
  TEST_ASSERT_EQUAL(2400000, adc.toVoltage(32768));  // 16-bit full scale
#else
  TestADC adc;
  adc.begin();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  adc.setReference(2.4f);
  adc.toVoltage(32768);
#endif
}

void test_to_voltage_gain2_half_scale() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_2, TestADC::BOOST_2);
  adc.setReference(2.4f);
  TEST_ASSERT_EQUAL(1200000, adc.toVoltage(32768));  // half scale
#else
  TestADC adc;
  adc.begin();
  adc.config2(false, TestADC::GAIN_2, TestADC::BOOST_2);
  adc.setReference(2.4f);
  adc.toVoltage(32768);
#endif
}

void test_to_voltage_gain033_triple() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_033, TestADC::BOOST_2);
  adc.setReference(2.4f);
  TEST_ASSERT_EQUAL(7200000, adc.toVoltage(32768));  // gain x1/3 → 3x
#else
  TestADC adc;
  adc.begin();
  adc.config2(false, TestADC::GAIN_033, TestADC::BOOST_2);
  adc.setReference(2.4f);
  adc.toVoltage(32768);
#endif
}

void test_to_voltage_24bit() {
#ifdef PIO_NATIVE_TESTING
  MCP3564 adc;
  adc.begin();
  setUp();
  adc.config2(false, MCP3564::GAIN_1, MCP3564::BOOST_2);
  adc.setReference(2.4f);
  TEST_ASSERT_EQUAL(2400000, adc.toVoltage(8388608));  // 24-bit full scale
#else
  TestADC adc;
  adc.begin();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  adc.setReference(2.4f);
  adc.toVoltage(32768);
#endif
}

// ---------------------------------------------------------------------------
// getMaxValue
// ---------------------------------------------------------------------------
void test_get_max_value_16bit() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  TEST_ASSERT_EQUAL(32768, adc.getMaxValue());
#else
  TestADC adc;
  adc.getMaxValue();
#endif
}

void test_get_max_value_24bit() {
#ifdef PIO_NATIVE_TESTING
  MCP3564 adc;
  TEST_ASSERT_EQUAL(8388608, adc.getMaxValue());
#else
  TestADC adc;
  adc.getMaxValue();
#endif
}

// ---------------------------------------------------------------------------
// setReference internal / external
// ---------------------------------------------------------------------------
void test_set_reference_internal() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.setReference(0.0);
  TEST_ASSERT_EQUAL(2.4f, adc.getReference());
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
  TEST_ASSERT(spiData[0][0] & 0x80);  // vref_sel = 1 → internal
#else
  TestADC adc;
  adc.begin();
  adc.setReference(0.0);
  adc.getReference();
#endif
}

void test_set_reference_external() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.setReference(3.3);
  TEST_ASSERT_EQUAL(3.3f, adc.getReference());
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
  TEST_ASSERT_FALSE(spiData[0][0] & 0x80);  // vref_sel = 0 → external
#else
  TestADC adc;
  adc.begin();
  adc.setReference(3.3);
  adc.getReference();
#endif
}

void test_get_reference_stored() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  adc.setReference(2.5);
  TEST_ASSERT_EQUAL(2.5f, adc.getReference());
#else
  TestADC adc;
  adc.begin();
  adc.setReference(2.5);
  adc.getReference();
#endif
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_status_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_status_dr_true);
  RUN_TEST(test_status_dr_false);
  RUN_TEST(test_status_crccfg);
  RUN_TEST(test_status_por);
  RUN_TEST(test_status_raw);
  RUN_TEST(test_is_complete_equals_status_dr);
  RUN_TEST(test_get_device_address_extracts_nibble);
  RUN_TEST(test_to_voltage_gain1_full_scale);
  RUN_TEST(test_to_voltage_gain2_half_scale);
  RUN_TEST(test_to_voltage_gain033_triple);
  RUN_TEST(test_to_voltage_24bit);
  RUN_TEST(test_get_max_value_16bit);
  RUN_TEST(test_get_max_value_24bit);
  RUN_TEST(test_set_reference_internal);
  RUN_TEST(test_set_reference_external);
  RUN_TEST(test_get_reference_stored);
#endif
}
