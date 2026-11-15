// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// analogRead() mux mode
// ---------------------------------------------------------------------------
void test_analogRead_mux_mode() {
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1 → exit loop immediately
  adc.begin();
  setUp();
  int32_t val = adc.analogRead(MCP3x6x_CH0);
  TEST_ASSERT_TRUE(spiAddrs.size() >= 3);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_CONVERSION, spiAddrs[1]);
}

// ---------------------------------------------------------------------------
// analogRead() mux mode — timeout
// ---------------------------------------------------------------------------
void test_analogRead_mux_mode_timeout() {
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1 → conversion completes immediately
  adc.begin();
  setUp();
  adc.setReadTimeout(1);  // timeout set but should not trigger (DR=1)
  int32_t val = adc.analogRead(MCP3x6x_CH0);
  TEST_ASSERT_NOT_EQUAL(MCP3x6x_TIMEOUT, val);
}

// ---------------------------------------------------------------------------
// analogRead() scan mode — returns cache, no SPI
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

  size_t addrCount = spiAddrs.size();
  adc.analogRead(MCP3x6x_CH0);
  TEST_ASSERT_EQUAL(addrCount, spiAddrs.size());  // no new SPI
}

// ---------------------------------------------------------------------------
// analogReadDifferential()
// ---------------------------------------------------------------------------
void test_analogRead_differential_waits_for_dr() {
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1 → exit loop immediately
  adc.begin();
  setUp();
  int32_t val = adc.analogReadDifferential(TestADC::MUX_CH0, TestADC::MUX_CH1);
  TEST_ASSERT_TRUE(spiAddrs.size() >= 3);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
  TEST_ASSERT_EQUAL(0x01, spiData[0][0]);  // VIN+=CH0, VIN-=CH1 → DIFFA=0x01
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_CONVERSION, spiAddrs[1]);
}

// ---------------------------------------------------------------------------
// readADCDATA — all data formats
// ---------------------------------------------------------------------------
void test_readADCDATA_sgn_data() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setDataFormat(TestADC::SGN_DATA);
  mockReadSequence = {0xFF, 0xF8};  // big-endian 16-bit value -8
  int32_t val = adc.readADCDATA();
  TEST_ASSERT_EQUAL(-8, val);
}

void test_readADCDATA_sgn_data_zero() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setDataFormat(TestADC::SGN_DATA_ZERO);
  mockReadSequence = {0x00, 0xFF, 0x00, 0x00};  // left-justified
  int32_t val = adc.readADCDATA();
  TEST_ASSERT_EQUAL(0xFF, val);  // raw >> 16
}

void test_readADCDATA_sgn_ext_data() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setDataFormat(TestADC::SGNEXT_DATA);
  mockReadSequence = {0x00, 0x00, 0x00, 0x64};  // value 100
  int32_t val = adc.readADCDATA();
  TEST_ASSERT_EQUAL(100, val);
}

// ---------------------------------------------------------------------------
// getLastChannel
// ---------------------------------------------------------------------------
void test_get_last_channel() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config3(false, false, false, TestADC::ID_SGNEXT_DATA, TestADC::CONTINUOUS);
  mockReadSequence = {0x20, 0x00, 0x00, 0x64};  // CH_ID=2
  adc.readADCDATA();
  TEST_ASSERT_EQUAL(2, (int)adc.getLastChannel());
}

// ---------------------------------------------------------------------------
// dataAvailable flag
// ---------------------------------------------------------------------------
void test_data_available_flag() {
  TestADC adc;
  mockStatusByte = 0x10;
  adc.begin();
  setUp();
  TEST_ASSERT_FALSE(adc.dataAvailable());

  mockStatusByte = 0x04;  // DR=1
  adc.IRQ_handler();
  TEST_ASSERT_TRUE(adc.dataAvailable());
}

// ---------------------------------------------------------------------------
// start/stop continuous
// ---------------------------------------------------------------------------
void test_start_continuous_sets_mode_and_converts() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.startContinuous();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_CONVERSION, spiAddrs[1]);
}

void test_stop_continuous_sets_standby() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.stopContinuous();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_STANDBY, spiAddrs[1]);
}

// ---------------------------------------------------------------------------
// startScanOnce
// ---------------------------------------------------------------------------
void test_start_scan_once_sets_oneshot_standby() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.startScanOnce();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[1]);
  // conv_mode = ONESHOT_STANDBY (2) in bits[7:6]
  TEST_ASSERT_EQUAL(2, (spiData[1][0] >> 6) & 0x03);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_CONVERSION, spiAddrs[3]);
}

// ---------------------------------------------------------------------------
// single/differential mode
// ---------------------------------------------------------------------------
void test_single_ended_mode_sets_vin_minus_agnd() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.singleEndedMode();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
}

void test_differential_mode_sets_ch0_ch1() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.differentialMode();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
}

void test_is_differential_true_false() {
  TestADC adc;
  adc.begin();
  setUp();
  TEST_ASSERT_FALSE(adc.isDifferential());
  adc.differentialMode();
  TEST_ASSERT_TRUE(adc.isDifferential());
}

// ---------------------------------------------------------------------------
// analogReadContinuous
// ---------------------------------------------------------------------------
void test_analog_read_continuous_not_continuous() {
  TestADC adc;
  adc.begin();
  setUp();
  TEST_ASSERT_EQUAL(-1, adc.analogReadContinuous(MCP3x6x_CH0));
}

void test_analog_read_continuous_cached() {
  TestADC adc;
  mockStatusByte = 0x10;
  adc.begin();
  setUp();
  adc.startContinuous();
  int32_t val = adc.analogReadContinuous(MCP3x6x_CH0);
  TEST_ASSERT_EQUAL(0, val);  // cached value is 0 (from mock)
}

// ---------------------------------------------------------------------------
// setReadTimeout / getReadTimeout
// ---------------------------------------------------------------------------
void test_set_read_timeout() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.setReadTimeout(100);
  TEST_ASSERT_EQUAL(100, adc.getReadTimeout());
}

// ---------------------------------------------------------------------------
// Fast commands
// ---------------------------------------------------------------------------
void test_conversion_fastcmd() {
  TestADC adc;
  setUp();
  adc.conversion();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_CONVERSION, spiAddrs[0]);
}

void test_standby_fastcmd() {
  TestADC adc;
  setUp();
  adc.standby();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_STANDBY, spiAddrs[0]);
}

void test_shutdown_fastcmd() {
  TestADC adc;
  setUp();
  adc.shutdown();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_SHUTDOWN, spiAddrs[0]);
}

void test_full_shutdown_fastcmd() {
  TestADC adc;
  setUp();
  adc.full_shutdown();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_FULL_SHUTDOWN, spiAddrs[0]);
}

void test_reset_fastcmd() {
  TestADC adc;
  setUp();
  adc.reset();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_RESET, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// IRQ_handler reads ADCDATA
// ---------------------------------------------------------------------------
void test_irq_handler_reads_adcdata() {
  TestADC adc;
  mockStatusByte = 0x10;
  adc.begin();
  setUp();
  mockStatusByte = 0x04;  // DR=1
  adc.IRQ_handler();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, spiAddrs[0]);
  TEST_ASSERT_TRUE(adc.dataAvailable());
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_conversion_tests() {
  RUN_TEST(test_analogRead_mux_mode);
  RUN_TEST(test_analogRead_mux_mode_timeout);
  RUN_TEST(test_analogRead_scan_mode_no_spi);
  RUN_TEST(test_analogRead_differential_waits_for_dr);
  RUN_TEST(test_readADCDATA_sgn_data);
  RUN_TEST(test_readADCDATA_sgn_data_zero);
  RUN_TEST(test_readADCDATA_sgn_ext_data);
  RUN_TEST(test_get_last_channel);
  RUN_TEST(test_data_available_flag);
  RUN_TEST(test_start_continuous_sets_mode_and_converts);
  RUN_TEST(test_stop_continuous_sets_standby);
  RUN_TEST(test_start_scan_once_sets_oneshot_standby);
  RUN_TEST(test_single_ended_mode_sets_vin_minus_agnd);
  RUN_TEST(test_differential_mode_sets_ch0_ch1);
  RUN_TEST(test_is_differential_true_false);
  RUN_TEST(test_analog_read_continuous_not_continuous);
  RUN_TEST(test_analog_read_continuous_cached);
  RUN_TEST(test_set_read_timeout);
  RUN_TEST(test_conversion_fastcmd);
  RUN_TEST(test_standby_fastcmd);
  RUN_TEST(test_shutdown_fastcmd);
  RUN_TEST(test_full_shutdown_fastcmd);
  RUN_TEST(test_reset_fastcmd);
  RUN_TEST(test_irq_handler_reads_adcdata);
}
