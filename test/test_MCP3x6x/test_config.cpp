// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#include "test.h"

#ifdef PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// configure() writes IRQ when value differs from default
// ---------------------------------------------------------------------------
void test_configure_writes_irq() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2, MCP3x6x_CFG_CONFIG3,
                0x00, MCP3x6x_CFG_MUX, MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER, MCP3x6x_CFG_OFFSET,
                MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// configure() writes MUX when value differs from default
// ---------------------------------------------------------------------------
void test_configure_writes_mux() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2, MCP3x6x_CFG_CONFIG3,
                MCP3x6x_CFG_IRQ, 0x01, MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER, MCP3x6x_CFG_OFFSET,
                MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// configure() writes LOCK when value differs from default
// ---------------------------------------------------------------------------
void test_configure_writes_lock() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2, MCP3x6x_CFG_CONFIG3,
                MCP3x6x_CFG_IRQ, MCP3x6x_CFG_MUX, MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER,
                MCP3x6x_CFG_OFFSET, MCP3x6x_CFG_GAIN, 0xA5, MCP3x6x_CFG_CRCCFG);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// configure() skips writes when all values match defaults
// ---------------------------------------------------------------------------
void test_configure_skips_unchanged() {
  TestADC adc;
  adc.begin();
  setUp();
  size_t prevCount = spiAddrs.size();
  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2, MCP3x6x_CFG_CONFIG3,
                MCP3x6x_CFG_IRQ, MCP3x6x_CFG_MUX, MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER,
                MCP3x6x_CFG_OFFSET, MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);
  TEST_ASSERT_EQUAL(prevCount, spiAddrs.size());
}

// ---------------------------------------------------------------------------
// config0() sends correct SPI address
// ---------------------------------------------------------------------------
void test_config0_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config0(TestADC::CONVERSION, TestADC::BIAS_0UA, TestADC::EXTERN, 0);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// config1() sends correct SPI address
// ---------------------------------------------------------------------------
void test_config1_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config1(TestADC::OSR_256, TestADC::MCLK_0);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// config2() sends correct SPI address
// ---------------------------------------------------------------------------
void test_config2_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// config3() sends correct SPI address
// ---------------------------------------------------------------------------
void test_config3_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config3(false, false, false, TestADC::SGN_DATA, TestADC::ONESHOT_SHUTDOWN);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// irq() sends correct SPI address
// ---------------------------------------------------------------------------
void test_irq_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.irq(false, false, 0);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// mux() sends correct SPI address
// ---------------------------------------------------------------------------
void test_mux_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.mux(TestADC::MUX_CH0, TestADC::MUX_CH1);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// scan() sends correct SPI address
// ---------------------------------------------------------------------------
void test_scan_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.scan(0x00, 0x00, false, false, false, false, TestADC::DLY_0);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// lock() sends correct SPI address and value
// ---------------------------------------------------------------------------
void test_lock_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.lock(0xA5);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(0xA5, spiData[0][0]);
}

// ---------------------------------------------------------------------------
// unlock() sends correct SPI address and value 0x00
// ---------------------------------------------------------------------------
void test_unlock_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.unlock();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(0x00, spiData[0][0]);
}

// ---------------------------------------------------------------------------
// write overloads verify correct SPI addressing
// ---------------------------------------------------------------------------
void test_write_Config0() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Config0<TestADC>(0x00));
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
}

void test_write_Config1() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Config1<TestADC>(0x00));
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
}

void test_write_Config2() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Config2<TestADC>(0x00));
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
}

void test_write_Config3() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Config3<TestADC>(0x00));
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG3, spiAddrs[0]);
}

void test_write_Irq() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Irq<TestADC>(0x00));
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, spiAddrs[0]);
}

void test_write_Mux() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Mux<TestADC>(0x00));
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[0]);
}

void test_write_Scan() {
  TestADC adc;
  adc.begin();
  setUp();
  uint8_t scanData[3] = {0x01, 0x02, 0x03};
  adc.write(MCP3x6xTest::Scan<TestADC>(scanData));
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, spiAddrs[0]);
}

void test_write_Lock() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.write(MCP3x6xTest::Lock<TestADC>(0xA5));
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// read overloads verify correct SPI addressing
// ---------------------------------------------------------------------------
void test_read_ADCDATA_SGN_DATA_16bit() {
  TestADC adc;
  adc.begin();
  setUp();
  MCP3x6xTest::Adcdata<TestADC> d;
  adc.read(&d);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, spiAddrs[0]);
}

void test_read_ADCDATA_SGN_DATA_24bit() {
  MCP3564 adc;
  adc.begin();
  setUp();
  MCP3x6xTest::Adcdata<MCP3564> d;
  adc.read(&d);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA, spiAddrs[0]);
}

void test_read_Config0() {
  TestADC adc;
  adc.begin();
  setUp();
  auto cfg0 = MCP3x6xTest::Config0<TestADC>(0x00);
  adc.read(cfg0);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG0, spiAddrs[0]);
}

void test_read_Config1() {
  TestADC adc;
  adc.begin();
  setUp();
  auto cfg1 = MCP3x6xTest::Config1<TestADC>(0x00);
  adc.read(cfg1);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IREAD | MCP3x6x_ADR_CONFIG1, spiAddrs[0]);
}

void test_read_Mux() {
  TestADC adc;
  adc.begin();
  setUp();
  auto mux = MCP3x6xTest::Mux<TestADC>(0x00);
  adc.read(mux);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IREAD | MCP3x6x_ADR_MUX, spiAddrs[0]);
}

// ---------------------------------------------------------------------------
// ADCDATA read byte count (SPI data transfer size)
// ---------------------------------------------------------------------------
void test_ADCDATA_read_16bit_2bytes() {
  TestADC adc;
  adc.begin();
  setUp();
  MCP3x6xTest::Adcdata<TestADC> d;
  adc.read(&d);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(2, spiData[0].size());
}

void test_ADCDATA_read_24bit_3bytes() {
  MCP3564 adc;
  adc.begin();
  setUp();
  MCP3x6xTest::Adcdata<MCP3564> d;
  adc.read(&d);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(3, spiData[0].size());
}

#endif  // PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_config_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_configure_writes_irq);
  RUN_TEST(test_configure_writes_mux);
  RUN_TEST(test_configure_writes_lock);
  RUN_TEST(test_configure_skips_unchanged);
  RUN_TEST(test_config0_writes_correct_addr);
  RUN_TEST(test_config1_writes_correct_addr);
  RUN_TEST(test_config2_writes_correct_addr);
  RUN_TEST(test_config3_writes_correct_addr);
  RUN_TEST(test_irq_writes_correct_addr);
  RUN_TEST(test_mux_writes_correct_addr);
  RUN_TEST(test_scan_writes_correct_addr);
  RUN_TEST(test_lock_writes_correct_addr);
  RUN_TEST(test_unlock_writes_correct_addr);
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
#endif
}
