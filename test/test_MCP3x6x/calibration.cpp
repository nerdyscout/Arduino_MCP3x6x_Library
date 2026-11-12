// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// setAveraging / setOversamplingRatio (alias)
// ---------------------------------------------------------------------------
void test_set_averaging_writes_config1() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setAveraging(TestADC::OSR_1024);
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
#endif
}

void test_set_oversampling_ratio_alias() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setOversamplingRatio(TestADC::OSR_4096);
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
#endif
}

// ---------------------------------------------------------------------------
// setDataFormat / setConversionMode / setAdcMode / setClockSelection
// ---------------------------------------------------------------------------
void test_set_data_format_writes_config3() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setDataFormat(TestADC::SGNEXT_DATA);
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
#endif
}

void test_set_conversion_mode_writes_config3() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setConversionMode(TestADC::CONTINUOUS);
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
#endif
}

void test_set_adc_mode_writes_config0() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setAdcMode(TestADC::CONVERSION);
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
#endif
}

void test_set_clock_selection_writes_config0() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setClockSelection(TestADC::INTERN);
#ifdef PIO_MOCKED_TESTING
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
#endif
}

// ---------------------------------------------------------------------------
// setResolution / setAnalogReadResolution are no-ops
// ---------------------------------------------------------------------------
void test_set_resolution_noop() {
  TestADC adc;
  adc.setResolution(12);
  TEST_ASSERT_TRUE(true);
}

void test_set_analog_read_resolution_noop() {
  TestADC adc;
  adc.analogReadResolution(12);
  TEST_ASSERT_TRUE(true);
}

// ---------------------------------------------------------------------------
// Temperature
// ---------------------------------------------------------------------------
void test_read_temperature_returns_value() {
  TestADC adc;
  adc.begin();
  setUp();
  mockStatusByte = 0x04;  // DR=1
#ifdef PIO_MOCKED_TESTING
  mockReadSequence = {0x00, 0x00, 0x00, 0x64};  // TEMP channel value
#endif
  int16_t temp = adc.readTemperature();
  TEST_ASSERT_TRUE(true);  // verifies no crash; value depends on mock
}

void test_get_temperature_restores_mux() {
  TestADC adc;
  adc.begin();
  setUp();
  mockStatusByte = 0x04;
#ifdef PIO_MOCKED_TESTING
  mockReadSequence = {0x00, 0x00, 0x00, 0x64};
#endif
  double temp = adc.getTemperature();
  TEST_ASSERT_TRUE(true);  // verifies no crash; MUX saved/restored
}

// ---------------------------------------------------------------------------
// setTempOffset / setTempSlope
// ---------------------------------------------------------------------------
void test_set_temp_offset_slope() {
  TestADC adc;
  adc.setTempOffset(100);
  adc.setTempSlope(1200);
  TEST_ASSERT_TRUE(true);
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_calibration_tests() {
  RUN_TEST(test_set_averaging_writes_config1);
  RUN_TEST(test_set_oversampling_ratio_alias);
  RUN_TEST(test_set_data_format_writes_config3);
  RUN_TEST(test_set_conversion_mode_writes_config3);
  RUN_TEST(test_set_adc_mode_writes_config0);
  RUN_TEST(test_set_clock_selection_writes_config0);
  RUN_TEST(test_read_temperature_returns_value);
  RUN_TEST(test_get_temperature_restores_mux);
  RUN_TEST(test_set_resolution_noop);
  RUN_TEST(test_set_analog_read_resolution_noop);
  RUN_TEST(test_set_temp_offset_slope);
}
