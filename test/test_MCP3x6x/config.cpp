// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// configure() — writes only registers whose raw value differs
// ---------------------------------------------------------------------------
void test_configure_skips_unchanged() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2, MCP3x6x_CFG_CONFIG3,
                MCP3x6x_CFG_IRQ, MCP3x6x_CFG_MUX, MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER,
                MCP3x6x_CFG_OFFSET, MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);
  // unlock + re-lock always occur; registers unchanged are skipped
  TEST_ASSERT_EQUAL(2, spiAddrs.size());
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[1]);
#else
  TestADC adc;
  adc.begin();
  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2, MCP3x6x_CFG_CONFIG3,
                MCP3x6x_CFG_IRQ, MCP3x6x_CFG_MUX, MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER,
                MCP3x6x_CFG_OFFSET, MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);
#endif
}

// ---------------------------------------------------------------------------
// configure() — always unlocks and re-locks
// ---------------------------------------------------------------------------
void test_configure_always_unlocks_relocks() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2, MCP3x6x_CFG_CONFIG3,
                0x00, MCP3x6x_CFG_MUX, MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER, MCP3x6x_CFG_OFFSET,
                MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);  // unlock
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ,   spiAddrs[1]);  // IRQ differs
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[2]);  // re-lock
#else
  TestADC adc;
  adc.begin();
  adc.unlock();
  adc.lock(0x00);
#endif
}

// ---------------------------------------------------------------------------
// Individual register write address verification
// ---------------------------------------------------------------------------
void test_config0_writes_correct_addr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config0(TestADC::CONVERSION, TestADC::BIAS_0UA, TestADC::EXTERN, 0);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.config0(TestADC::CONVERSION, TestADC::BIAS_0UA, TestADC::EXTERN, 0);
#endif
}

void test_config1_writes_correct_addr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config1(TestADC::OSR_256, TestADC::MCLK_0);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.config1(TestADC::OSR_256, TestADC::MCLK_0);
#endif
}

void test_config2_writes_correct_addr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
#endif
}

void test_config3_writes_correct_addr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config3(false, false, false, TestADC::SGN_DATA, TestADC::ONESHOT_SHUTDOWN);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.config3(false, false, false, TestADC::SGN_DATA, TestADC::ONESHOT_SHUTDOWN);
#endif
}

void test_irq_writes_correct_addr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.irq(false, false, TestADC::IRQ_OFF);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.irq(false, false, TestADC::IRQ_OFF);
#endif
}

void test_mux_writes_correct_addr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.mux(TestADC::MUX_CH0, TestADC::MUX_CH1);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.mux(TestADC::MUX_CH0, TestADC::MUX_CH1);
#endif
}

void test_scan_writes_correct_addr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.scan(0x00, 0x00, false, false, false, false, TestADC::DLY_0);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.scan(0x00, 0x00, false, false, false, false, TestADC::DLY_0);
#endif
}

// ---------------------------------------------------------------------------
// lock / unlock
// ---------------------------------------------------------------------------
void test_lock_writes_correct_addr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.lock(0xA5);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(0xA5, spiData[0][0]);
#else
  TestADC adc;
  adc.begin();
  adc.lock(0xA5);
#endif
}

void test_unlock_writes_correct_addr() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.unlock();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(0xA5, spiData[0][0]);  // unlock key
#else
  TestADC adc;
  adc.begin();
  adc.unlock();
#endif
}

void test_lock_default_writes_lock() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.lock();
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(0x00, spiData[0][0]);
#else
  TestADC adc;
  adc.begin();
  adc.lock();
#endif
}

// ---------------------------------------------------------------------------
// write() overloads — verify correct SPI addressing
// ---------------------------------------------------------------------------
void test_write_Config0() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Config0<TestADC>(0x00));
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.write(MCP3x6xTest::Config0<TestADC>(0x00));
#endif
}

void test_write_Config1() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Config1<TestADC>(0x00));
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.write(MCP3x6xTest::Config1<TestADC>(0x00));
#endif
}

void test_write_Config2() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Config2<TestADC>(0x00));
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT(spiData[0][0] & 0x01);  // datasheet requires bit 0 = 1
#else
  TestADC adc;
  adc.begin();
  adc.write(MCP3x6xTest::Config2<TestADC>(0x00));
#endif
}

void test_write_Config3() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Config3<TestADC>(0x00));
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.write(MCP3x6xTest::Config3<TestADC>(0x00));
#endif
}

void test_write_Irq() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Irq<TestADC>(0x00));
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.write(MCP3x6xTest::Irq<TestADC>(0x00));
#endif
}

void test_write_Mux() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Mux<TestADC>(0x00));
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.write(MCP3x6xTest::Mux<TestADC>(0x00));
#endif
}

void test_write_Scan() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  uint8_t scanData[3] = {0xFF, 0xFF, 0xFF};
  adc.write(MCP3x6xTest::Scan<TestADC>(scanData));
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, spiAddrs[0]);
  // reserved bit 20 (bit 4 of byte 2) must be forced to 0
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(3, spiData[0].size());
  TEST_ASSERT_EQUAL(0x00, spiData[0][0] & 0x10);  // reserved bit cleared in byte 2
#else
  TestADC adc;
  adc.begin();
  uint8_t scanData[3] = {0xFF, 0xFF, 0xFF};
  adc.write(MCP3x6xTest::Scan<TestADC>(scanData));
#endif
}

void test_write_Lock() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Lock<TestADC>(0xA5));
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  adc.write(MCP3x6xTest::Lock<TestADC>(0xA5));
#endif
}

// ---------------------------------------------------------------------------
// read() overloads — verify correct SPI addressing
// ---------------------------------------------------------------------------
void test_read_ADCDATA_SGN_DATA_16bit() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  MCP3x6xTest::Adcdata<TestADC> d;
  adc.read(&d);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
  // hardware: read not validated via mock
#endif
}

void test_read_ADCDATA_SGN_DATA_24bit() {
#ifdef PIO_NATIVE_TESTING
  mockReadSequence = {0x0F, 0x30};  // HW_ID=MCP3564, R-variant
  MCP3564 adc;
  adc.begin();
  setUp();
  MCP3x6xTest::Adcdata<MCP3564> d;
  adc.read(&d);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
#endif
}

void test_read_Config0() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  auto cfg0 = MCP3x6xTest::Config0<TestADC>(0x00);
  adc.read(cfg0);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
#endif
}

void test_read_Config1() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  auto cfg1 = MCP3x6xTest::Config1<TestADC>(0x00);
  adc.read(cfg1);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
#endif
}

void test_read_Mux() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  auto mux = MCP3x6xTest::Mux<TestADC>(0x00);
  adc.read(mux);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX, spiAddrs[0]);
#else
  TestADC adc;
  adc.begin();
#endif
}

// ---------------------------------------------------------------------------
// ADCDATA read byte count
// ---------------------------------------------------------------------------
void test_ADCDATA_read_16bit_2bytes() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  MCP3x6xTest::Adcdata<TestADC> d;
  adc.read(&d);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(2, spiData[0].size());
#else
  TestADC adc;
  adc.begin();
#endif
}

void test_ADCDATA_read_24bit_3bytes() {
#ifdef PIO_NATIVE_TESTING
  mockReadSequence = {0x0F, 0x30};  // HW_ID=MCP3564, R-variant
  MCP3564 adc;
  adc.begin();
  setUp();
  MCP3x6xTest::Adcdata<MCP3564> d;
  adc.read(&d);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(3, spiData[0].size());
#else
  TestADC adc;
  adc.begin();
#endif
}

// ---------------------------------------------------------------------------
// readADCDATA — channel strip + sign extension (ID_SGNEXT_DATA)
// ---------------------------------------------------------------------------
void test_readADCDATA_id_sgnnext_strips_channel() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config3(false, false, false, TestADC::ID_SGNEXT_DATA, TestADC::CONTINUOUS);
  mockReadSequence = {0x20, 0x00, 0x00, 0x64};  // CH_ID=2, value=100
  int32_t val = adc.readADCDATA();
  TEST_ASSERT_EQUAL(100, val);
  TEST_ASSERT_EQUAL(2, (int)adc.getLastChannel());
#else
  TestADC adc;
  adc.begin();
  adc.config3(false, false, false, TestADC::ID_SGNEXT_DATA, TestADC::CONTINUOUS);
  adc.readADCDATA();
#endif
}

void test_readADCDATA_id_sgnnext_negative_value() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config3(false, false, false, TestADC::ID_SGNEXT_DATA, TestADC::CONTINUOUS);
  mockReadSequence = {0x5F, 0xFF, 0xFF, 0xFF};  // CH_ID=5, value=-1
  int32_t val = adc.readADCDATA();
  TEST_ASSERT_EQUAL(-1, val);
  TEST_ASSERT_EQUAL(5, (int)adc.getLastChannel());
#else
  TestADC adc;
  adc.begin();
  adc.config3(false, false, false, TestADC::ID_SGNEXT_DATA, TestADC::CONTINUOUS);
  adc.readADCDATA();
#endif
}

void test_readADCDATA_sgn_data_unchanged() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  mockReadSequence = {0xFF, 0xF8};  // big-endian 16-bit value -8
  int32_t val = adc.readADCDATA();
  TEST_ASSERT_EQUAL(-8, val);
#else
  TestADC adc;
  adc.begin();
  adc.readADCDATA();
#endif
}

// ---------------------------------------------------------------------------
// config2() az_ref / BOOST / GAIN
// ---------------------------------------------------------------------------
void test_config2_az_ref_true() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2, true);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT(spiData[0][0] & 0x02);  // az_ref = bit 1
#else
  TestADC adc;
  adc.begin();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2, true);
#endif
}

void test_config2_az_ref_default_false() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_FALSE(spiData[0][0] & 0x02);
#else
  TestADC adc;
  adc.begin();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
#endif
}

void test_config2_boost_gain_boost2() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(0x89, spiData[0][0]);  // BOOST=2, GAIN=1, RESERVED=1
#else
  TestADC adc;
  adc.begin();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
#endif
}

// ---------------------------------------------------------------------------
// IRQ enum values
// ---------------------------------------------------------------------------
void test_irq_enum_values() {
#ifdef PIO_NATIVE_TESTING
  TEST_ASSERT_EQUAL(0, (uint8_t)TestADC::IRQ_OFF);
  TEST_ASSERT_EQUAL(1, (uint8_t)TestADC::IRQ_OPEN_DRAIN);
  TEST_ASSERT_EQUAL(2, (uint8_t)TestADC::IRQ_PUSH_PULL_LOW);
  TEST_ASSERT_EQUAL(3, (uint8_t)TestADC::IRQ_PUSH_PULL_HIGH);
#else
  // Hardware: enum values verified by native.
#endif
}

void test_irq_enum_written() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.irq(false, true, TestADC::IRQ_PUSH_PULL_HIGH);
  TEST_ASSERT_EQUAL_HEX8(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(3, (spiData[0][0] >> 2) & 0x03);  // irq_mode = 3
#else
  TestADC adc;
  adc.begin();
  adc.irq(false, true, TestADC::IRQ_PUSH_PULL_HIGH);
#endif
}

// ---------------------------------------------------------------------------
// getDeviceAddress
// ---------------------------------------------------------------------------
void test_getDeviceAddress() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x30;  // STATUS bits [5:3] = 110 → DEV_ADDR = 3
  adc.begin();
  TEST_ASSERT_EQUAL(3, adc.getDeviceAddress());
#else
  TestADC adc;
  adc.begin();
  adc.getDeviceAddress();
#endif
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_config_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_configure_skips_unchanged);
  RUN_TEST(test_configure_always_unlocks_relocks);
  RUN_TEST(test_config0_writes_correct_addr);
  RUN_TEST(test_config1_writes_correct_addr);
  RUN_TEST(test_config2_writes_correct_addr);
  RUN_TEST(test_config3_writes_correct_addr);
  RUN_TEST(test_irq_writes_correct_addr);
  RUN_TEST(test_mux_writes_correct_addr);
  RUN_TEST(test_scan_writes_correct_addr);
  RUN_TEST(test_lock_writes_correct_addr);
  RUN_TEST(test_unlock_writes_correct_addr);
  RUN_TEST(test_lock_default_writes_lock);
  RUN_TEST(test_write_Config0);
  RUN_TEST(test_write_Config1);
  RUN_TEST(test_write_Config2);
  RUN_TEST(test_write_Config3);
  RUN_TEST(test_write_Irq);
  RUN_TEST(test_write_Mux);
  RUN_TEST(test_write_Scan);
  RUN_TEST(test_write_Lock);
  RUN_TEST(test_read_ADCDATA_SGN_DATA_16bit);
  RUN_TEST(test_read_ADCDATA_SGN_DATA_24bit);
  RUN_TEST(test_read_Config0);
  RUN_TEST(test_read_Config1);
  RUN_TEST(test_read_Mux);
  RUN_TEST(test_ADCDATA_read_16bit_2bytes);
  RUN_TEST(test_ADCDATA_read_24bit_3bytes);
  RUN_TEST(test_readADCDATA_id_sgnnext_strips_channel);
  RUN_TEST(test_readADCDATA_id_sgnnext_negative_value);
  RUN_TEST(test_readADCDATA_sgn_data_unchanged);
  RUN_TEST(test_config2_az_ref_true);
  RUN_TEST(test_config2_az_ref_default_false);
  RUN_TEST(test_config2_boost_gain_boost2);
  RUN_TEST(test_irq_enum_values);
  RUN_TEST(test_irq_enum_written);
  RUN_TEST(test_getDeviceAddress);
#endif
}
