// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// status_* getters (inverted logic: status_dr() = false when DR=1 / data ready)
// ---------------------------------------------------------------------------
void test_status_dr_true() {
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1 → status_dr() = false (data ready)
  adc.reset();
  TEST_ASSERT_FALSE(adc.status_dr());
}

void test_status_dr_false() {
  TestADC adc;
  mockStatusByte = 0x00;  // DR=0 → status_dr() = true (data not ready)
  adc.reset();
  TEST_ASSERT_TRUE(adc.status_dr());
}

void test_status_crccfg() {
  TestADC adc;
  mockStatusByte = 0x02;  // CRCCFG=1 (bit 1) → inverted → false
  adc.reset();
  TEST_ASSERT_FALSE(adc.status_crccfg());
}

void test_status_por() {
  TestADC adc;
  mockStatusByte = 0x01;  // POR=1 (bit 0) → inverted → false
  adc.reset();
  TEST_ASSERT_FALSE(adc.status_por());
}

void test_status_raw() {
  TestADC adc;
  mockStatusByte = 0xAB;
  adc.reset();
  TEST_ASSERT_EQUAL_HEX8(0xAB, adc.status());
}

// ---------------------------------------------------------------------------
// isComplete delegates to status_dr
// ---------------------------------------------------------------------------
void test_is_complete_equals_status_dr() {
  TestADC adc;
  mockStatusByte = 0x04;
  adc.reset();
  TEST_ASSERT_EQUAL(adc.status_dr(), adc.isComplete());
}

// ---------------------------------------------------------------------------
// getDeviceAddress
// ---------------------------------------------------------------------------
void test_get_device_address_extracts_nibble() {
  TestADC adc;
  mockStatusByte = 0x30;  // bits [5:3] = 110 → DEV_ADDR[1:0] = 3
  adc.begin();
  TEST_ASSERT_EQUAL(3, adc.getDeviceAddress());
}

// ---------------------------------------------------------------------------
// toVoltage — datasheet conversion V_uV = raw * VREF / 2^(N-1) / gain
// ---------------------------------------------------------------------------
void test_to_voltage_gain1_full_scale() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  adc.setReference(2.4f);
  TEST_ASSERT_EQUAL(2400000, adc.toVoltage(32768));  // 16-bit full scale
}

void test_to_voltage_gain2_half_scale() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_2, TestADC::BOOST_2);
  adc.setReference(2.4f);
  TEST_ASSERT_EQUAL(1200000, adc.toVoltage(32768));  // half scale
}

void test_to_voltage_gain033_triple() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_033, TestADC::BOOST_2);
  adc.setReference(2.4f);
  TEST_ASSERT_EQUAL(7200000, adc.toVoltage(32768));  // gain x1/3 → 3x
}

void test_to_voltage_24bit() {
  MCP3564 adc;
  adc.begin();
  setUp();
  adc.config2(false, MCP3564::GAIN_1, MCP3564::BOOST_2);
  adc.setReference(2.4f);
  TEST_ASSERT_EQUAL(2400000, adc.toVoltage(8388608));  // 24-bit full scale
}

// ---------------------------------------------------------------------------
// getMaxValue
// ---------------------------------------------------------------------------
void test_get_max_value_16bit() {
  TestADC adc;
  TEST_ASSERT_EQUAL(32768, adc.getMaxValue());
}

void test_get_max_value_24bit() {
  MCP3564 adc;
  TEST_ASSERT_EQUAL(8388608, adc.getMaxValue());
}

// ---------------------------------------------------------------------------
// setReference internal / external
// ---------------------------------------------------------------------------
void test_set_reference_internal() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setReference(0.0);
  TEST_ASSERT_EQUAL(2.4f, adc.getReference());
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
  TEST_ASSERT(spiData[0][0] & 0x80);  // vref_sel = 1 → internal
#endif
}

void test_set_reference_external() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setReference(3.3);
  TEST_ASSERT_EQUAL(3.3f, adc.getReference());
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
  TEST_ASSERT_FALSE(spiData[0][0] & 0x80);  // vref_sel = 0 → external
#endif
}

void test_get_reference_stored() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setReference(2.5);
  TEST_ASSERT_EQUAL(2.5f, adc.getReference());
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_status_tests() {
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
}
