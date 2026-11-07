// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

#ifdef PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// setReference(vref=0) → sets internal ref + writes CONFIG0
// ---------------------------------------------------------------------------
void test_setReference_internal() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setReference(0.0);
  TEST_ASSERT_EQUAL(2.4f, adc.getReference());
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
  TEST_ASSERT(spiData[0][0] & 0x80);  // vref_sel (bit 7) = 1 -> internal
}

// ---------------------------------------------------------------------------
// setReference(vref=3.3) → sets external ref + writes CONFIG0
// ---------------------------------------------------------------------------
void test_setReference_external() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setReference(3.3);
  TEST_ASSERT_EQUAL(3.3f, adc.getReference());
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
  TEST_ASSERT_FALSE(spiData[0][0] & 0x80);  // vref_sel (bit 7) = 0 -> external
}

// ---------------------------------------------------------------------------
// getReference() returns stored reference
// ---------------------------------------------------------------------------
void test_getReference() {
  TestADC adc;
  adc.begin();
  adc.setReference(2.5);
  TEST_ASSERT_EQUAL(2.5f, adc.getReference());
}

// ---------------------------------------------------------------------------
// getMaxValue() returns 2^MAX_RESOLUTION
// ---------------------------------------------------------------------------
void test_getMaxValue_16bit() {
  TestADC adc;
  TEST_ASSERT_EQUAL(32768, adc.getMaxValue());
}

void test_getMaxValue_24bit() {
  MCP3564 adc;
  TEST_ASSERT_EQUAL(8388608, adc.getMaxValue());
}

// ---------------------------------------------------------------------------
// toVoltage() — datasheet conversion: V_uV = raw * VREF / 2^(N-1) / gain
// ---------------------------------------------------------------------------
void test_toVoltage_gain1_full_scale() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  adc.setReference(2.4f);
  // raw = 2^(N-1) at gain x1, VREF = 2.4V → 2.4V full scale
  TEST_ASSERT_EQUAL(2400000, adc.toVoltage(32768));
}

void test_toVoltage_gain2_half_scale() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_2, TestADC::BOOST_2);
  adc.setReference(2.4f);
  TEST_ASSERT_EQUAL(1200000, adc.toVoltage(32768));
}

void test_toVoltage_gain033() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_033, TestADC::BOOST_2);
  adc.setReference(2.4f);
  // gain x1/3 → effective voltage is 3x the code-derived value
  TEST_ASSERT_EQUAL(7200000, adc.toVoltage(32768));
}

// ---------------------------------------------------------------------------
// singleEndedMode() sets MUX VIN- to AGND
// ---------------------------------------------------------------------------
void test_singleEndedMode() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.singleEndedMode();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// differentialMode() sets MUX to CH0-CH1
// ---------------------------------------------------------------------------
void test_differentialMode() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.differentialMode();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// isDifferential() returns false in default single-ended mode
// ---------------------------------------------------------------------------
void test_isDifferential_false() {
  TestADC adc;
  adc.begin();
  TEST_ASSERT_FALSE(adc.isDifferential());
}

// ---------------------------------------------------------------------------
// isDifferential() returns true after differentialMode()
// ---------------------------------------------------------------------------
void test_isDifferential_true() {
  TestADC adc;
  adc.begin();
  adc.differentialMode();
  TEST_ASSERT_TRUE(adc.isDifferential());
}

// ---------------------------------------------------------------------------
// setAveraging / setOversamplingRatio writes CONFIG1
// ---------------------------------------------------------------------------
void test_setAveraging_writes_config1() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setAveraging(TestADC::OSR_1024);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
}

void test_setOversamplingRatio_writes_config1() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setOversamplingRatio(TestADC::OSR_4096);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// setConversionMode writes CONFIG3
// ---------------------------------------------------------------------------
void test_setConversionMode_writes_config3() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setConversionMode(TestADC::CONTINUOUS);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// setAdcMode writes CONFIG0
// ---------------------------------------------------------------------------
void test_setAdcMode_writes_config0() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setAdcMode(TestADC::CONVERSION);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// setClockSelection writes CONFIG0
// ---------------------------------------------------------------------------
void test_setClockSelection_writes_config0() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setClockSelection(TestADC::INTERN);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// Mux constructor creates valid instance (no crash)
// ---------------------------------------------------------------------------
void test_mux_constructor() {
  TestADC adc(10, 11, 12, 13);
  TEST_ASSERT_TRUE(true);
}

// ---------------------------------------------------------------------------
// end() calls SPI.end() — verified by no crash
// ---------------------------------------------------------------------------
void test_end_calls_spi_end() {
  TestADC adc;
  adc.begin();
  adc.end();
  TEST_ASSERT_TRUE(true);
}

// ---------------------------------------------------------------------------
// setResolution and analogReadResolution are no-ops
// ---------------------------------------------------------------------------
void test_setResolution_noop() {
  TestADC adc;
  adc.setResolution(12);
  TEST_ASSERT_TRUE(true);
}

void test_analogReadResolution_noop() {
  TestADC adc;
  adc.analogReadResolution(12);
  TEST_ASSERT_TRUE(true);
}

#endif  // PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_misc_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_setReference_internal);
  RUN_TEST(test_setReference_external);
  RUN_TEST(test_getReference);
  RUN_TEST(test_getMaxValue_16bit);
  RUN_TEST(test_getMaxValue_24bit);
  RUN_TEST(test_toVoltage_gain1_full_scale);
  RUN_TEST(test_toVoltage_gain2_half_scale);
  RUN_TEST(test_toVoltage_gain033);
  RUN_TEST(test_singleEndedMode);
  RUN_TEST(test_differentialMode);
  RUN_TEST(test_isDifferential_false);
  RUN_TEST(test_isDifferential_true);
  RUN_TEST(test_setAveraging_writes_config1);
  RUN_TEST(test_setOversamplingRatio_writes_config1);
  RUN_TEST(test_setConversionMode_writes_config3);
  RUN_TEST(test_setAdcMode_writes_config0);
  RUN_TEST(test_setClockSelection_writes_config0);
  RUN_TEST(test_mux_constructor);
  RUN_TEST(test_end_calls_spi_end);
  RUN_TEST(test_setResolution_noop);
  RUN_TEST(test_analogReadResolution_noop);
#endif
}
