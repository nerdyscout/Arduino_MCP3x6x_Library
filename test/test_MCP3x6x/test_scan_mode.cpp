// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#include "__init__.h"

#ifdef PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// enableScanChannel() sets correct bit in SCAN register
// ---------------------------------------------------------------------------
void test_enable_scan_channel_sets_bit() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  // CH0 has _channelID[0] = MCP3x6x_CH0 = 0x08
  // bitSet(_scan.channel.raw, 0) → LSB set → SCAN byte 0 = 0x01
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(3, spiData[0].size());
  // SCAN bit 0 set for CH0 (last byte after byte-reversal for big-endian wire format)
  TEST_ASSERT(spiData[0][2] & 0x01);
}

// ---------------------------------------------------------------------------
// disableScanChannel() clears correct bit in SCAN register
// ---------------------------------------------------------------------------
void test_disable_scan_channel_clears_bit() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  setUp();
  adc.disableScanChannel(MCP3x6x_CH0);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, spiAddrs[0]);
  TEST_ASSERT_FALSE(spiData[0][2] & 0x01);
}

// ---------------------------------------------------------------------------
// startScan() configures IRQ + ADC mode + conversion
// ---------------------------------------------------------------------------
void test_startScan_configures_irq_adc_conversion() {
  TestADC adc;
  adc.begin();
  setUp();

  adc.startScan();
  // 1: write IRQ (en_fastcmd=1, irq_mode=01)
  // 2: write CONFIG3 (ID_SGNEXT_DATA)
  // 3: write CONFIG0 (adc=CONVERSION)
  // 4: fastcmd CONVERSION
  TEST_ASSERT(spiAddrs.size() >= 4);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, spiAddrs[0]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[1]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[2]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_CONVERSION, spiAddrs[3]);
}

// ---------------------------------------------------------------------------
// startContinuous() sets conversion mode to CONTINUOUS + conversion
// ---------------------------------------------------------------------------
void test_startContinuous_sets_continuous() {
  TestADC adc;
  adc.begin();
  setUp();

  adc.startContinuous();
  // write CONFIG3 with conv_mode=CONTINUOUS, then conversion fastcmd
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_CONVERSION, spiAddrs[1]);
}

// ---------------------------------------------------------------------------
// stopContinuous() sets ONESHOT_STANDBY + standby fastcmd
// ---------------------------------------------------------------------------
void test_stopContinuous_sets_standby() {
  TestADC adc;
  adc.begin();
  setUp();

  adc.stopContinuous();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_STANDBY, spiAddrs[1]);
}

// ---------------------------------------------------------------------------
// isContinuous() returns true after startContinuous
// ---------------------------------------------------------------------------
void test_isContinuous_true() {
  TestADC adc;
  adc.begin();
  adc.startContinuous();
  TEST_ASSERT_TRUE(adc.isContinuous());
}

// ---------------------------------------------------------------------------
// isContinuous() returns false in default mode
// ---------------------------------------------------------------------------
void test_isContinuous_false() {
  TestADC adc;
  adc.begin();
  TEST_ASSERT_FALSE(adc.isContinuous());
}

// ---------------------------------------------------------------------------
// startContinuousDifferential() sets diff mode + continuous
// ---------------------------------------------------------------------------
void test_startContinuousDifferential() {
  TestADC adc;
  adc.begin();
  setUp();

  adc.startContinuousDifferential();
  // write MUX (differentialMode), then write CONFIG3 (continuous), then conversion
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[1]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_CONVERSION, spiAddrs[2]);
}

#endif  // PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_scan_mode_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_enable_scan_channel_sets_bit);
  RUN_TEST(test_disable_scan_channel_clears_bit);
  RUN_TEST(test_startScan_configures_irq_adc_conversion);
  RUN_TEST(test_startContinuous_sets_continuous);
  RUN_TEST(test_stopContinuous_sets_standby);
  RUN_TEST(test_isContinuous_true);
  RUN_TEST(test_isContinuous_false);
  RUN_TEST(test_startContinuousDifferential);
#endif
}
