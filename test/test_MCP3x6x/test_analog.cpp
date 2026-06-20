// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#include "__init__.h"

#ifdef PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// analogRead() mux mode: writes MUX, conversion, waits for DR, returns value
// ---------------------------------------------------------------------------
void test_analogRead_mux_mode() {
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1 → exit loop immediately
  adc.begin();
  setUp();

  int32_t val = adc.analogRead(MCP3x6x_CH0);
  // Should have written MUX, sent conversion, read ADCDATA
  TEST_ASSERT_TRUE(spiAddrs.size() >= 3);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_CONVERSION, spiAddrs[1]);
}

// ---------------------------------------------------------------------------
// analogRead() scan mode: returns value from cache without SPI calls
// ---------------------------------------------------------------------------
void test_analogRead_scan_mode_no_spi() {
  TestADC adc;
  mockStatusByte = 0x10;
  adc.begin();
  setUp();

  adc.enableScanChannel(MCP3x6x_CH0);
  adc.mux(TestADC::MUX_CH0, TestADC::MUX_AGND);
  adc.startScan();
  mockStatusByte = 0x04;  // DR=1
  adc.IRQ_handler();

  // analogRead scan mode should NOT produce any SPI traffic
  size_t addrCount = spiAddrs.size();
  adc.analogRead(MCP3x6x_CH0);
  TEST_ASSERT_EQUAL(addrCount, spiAddrs.size());
}

// ---------------------------------------------------------------------------
// analogReadDifferential() waits for DR flag
// ---------------------------------------------------------------------------
void test_analogRead_differential_waits_for_dr() {
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1 → exit loop immediately
  adc.begin();
  setUp();

  int32_t val = adc.analogReadDifferential(MCP3x6x_CH0, MCP3x6x_CH1);
  // Should have at least: write MUX + conversion + read ADCDATA
  TEST_ASSERT(spiAddrs.size() >= 3);
}

// ---------------------------------------------------------------------------
// analogReadContinuous() returns -1 when not in continuous mode
// ---------------------------------------------------------------------------
void test_analogReadContinuous_not_continuous() {
  TestADC adc;
  adc.begin();
  setUp();
  int32_t val = adc.analogReadContinuous(MCP3x6x_CH0);
  TEST_ASSERT_EQUAL(-1, val);
}

// ---------------------------------------------------------------------------
// analogReadContinuous() returns cached value in continuous mode
// ---------------------------------------------------------------------------
void test_analogReadContinuous_continuous_mode() {
  TestADC adc;
  mockStatusByte = 0x10;
  adc.begin();
  adc.startContinuous();
  int32_t val = adc.analogReadContinuous(MCP3x6x_CH0);
  TEST_ASSERT_EQUAL(0, val);  // cached value is 0 (from mock)
}

// ---------------------------------------------------------------------------
// IRQ_handler() reads ADCDATA
// ---------------------------------------------------------------------------
void test_IRQ_handler_reads_adcdata() {
  TestADC adc;
  mockStatusByte = 0x10;
  adc.begin();
  setUp();
  mockStatusByte = 0x04;  // DR=1
  adc.IRQ_handler();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, spiAddrs[0]);
  TEST_ASSERT_TRUE(adc.available());
}

#endif  // PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_analog_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_analogRead_mux_mode);
  RUN_TEST(test_analogRead_scan_mode_no_spi);
  RUN_TEST(test_analogRead_differential_waits_for_dr);
  RUN_TEST(test_analogReadContinuous_not_continuous);
  RUN_TEST(test_analogReadContinuous_continuous_mode);
  RUN_TEST(test_IRQ_handler_reads_adcdata);
#endif
}
