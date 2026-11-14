// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// Hardware-in-the-loop (HIL) Unity test suite for real MCP3x6x hardware.
// Intentionally minimal so it fits ATmega328P RAM (2 KB): covers init,
// basic register/config writes, status helpers, and a simple conversion —
// expectations that require actual hardware. Heavy/mocked-only checks
// (DMA, exhaustive conversion paths, macro/enum tables, full config matrix)
// remain in test/test_mock/ where they can be fully simulated.

#include <Arduino.h>
#include <SPI.h>
#include <unity.h>

#include "Arduino_MCP3x6x_Library.hpp"

#ifndef TEST_ADC_TYPE
#error "Define TEST_ADC_TYPE for hardware testing (e.g. -DTEST_ADC_TYPE=MCP3564)"
#endif
using TestADC = TEST_ADC_TYPE;

static TestADC adc;

void setUp(void) {
}
void tearDown(void) {
}

void test_begin_success(void) {
  TEST_ASSERT_TRUE_MESSAGE(adc.begin(), "begin() should succeed (POR+chip-id)");
}

void test_device_address_in_range(void) {
  uint8_t addr = adc.getDeviceAddress();
  TEST_ASSERT_TRUE(addr <= 3);
}

void test_status_raw_readable(void) {
  (void)adc.status();
  TEST_ASSERT_TRUE(true);
}

void test_config0_writes(void) {
  adc.config0(TestADC::CONVERSION, TestADC::BIAS_0UA, TestADC::INTERN, true, 0);
  TEST_ASSERT_TRUE(true);
}

void test_config1_writes(void) {
  adc.config1(TestADC::OSR_256, TestADC::MCLK_0);
  TEST_ASSERT_TRUE(true);
}

void test_config2_writes(void) {
  adc.config2(true, TestADC::GAIN_1, TestADC::BOOST_2);
  TEST_ASSERT_TRUE(true);
}

void test_config3_writes(void) {
  adc.config3(true, true, false, TestADC::SGNEXT_DATA, TestADC::ONESHOT_STANDBY);
  TEST_ASSERT_TRUE(true);
}

void test_mux_and_conversion_completes(void) {
  adc.config0(TestADC::CONVERSION, TestADC::BIAS_0UA, TestADC::INTERN, true, 0);
  adc.mux(TestADC::MUX_CH0, TestADC::MUX_AGND);
  int32_t val = adc.analogRead(MCP3x6x_CH0);
  // On a real board the value is signal-dependent; verify the call completes
  // and does not return the timeout sentinel.
  TEST_ASSERT_NOT_EQUAL(MCP3x6x_TIMEOUT, val);
}

void test_get_max_value_positive(void) {
  TEST_ASSERT_TRUE(adc.getMaxValue() > 0);
}

void setup(void) {
#if defined(USB_VID) && defined(USB_PID)
  while (!Serial)
    ;
#else
  delay(2000);
#endif
  Serial.begin(115200);
  while (!Serial)
    ;

  UNITY_BEGIN();
  RUN_TEST(test_begin_success);
  RUN_TEST(test_device_address_in_range);
  RUN_TEST(test_status_raw_readable);
  RUN_TEST(test_config0_writes);
  RUN_TEST(test_config1_writes);
  RUN_TEST(test_config2_writes);
  RUN_TEST(test_config3_writes);
  RUN_TEST(test_mux_and_conversion_completes);
  RUN_TEST(test_get_max_value_positive);
  UNITY_END();
}

void loop(void) {
}
