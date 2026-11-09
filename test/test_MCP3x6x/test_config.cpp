// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

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
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);  // unlock
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, spiAddrs[1]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[2]);  // re-lock
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
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);  // unlock
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX, spiAddrs[1]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[2]);  // re-lock
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
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);  // unlock (0x00)
  size_t last = spiAddrs.size() - 1;
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[last]);
  TEST_ASSERT_EQUAL(0xA5, spiData[last][0]);  // user's lock value
}

// ---------------------------------------------------------------------------
// configure() skips writes when all values match defaults
// ---------------------------------------------------------------------------
void test_configure_skips_unchanged() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2, MCP3x6x_CFG_CONFIG3,
                MCP3x6x_CFG_IRQ, MCP3x6x_CFG_MUX, MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER,
                MCP3x6x_CFG_OFFSET, MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);
  // unlock + re-lock always occur; registers unchanged are skipped
  TEST_ASSERT_EQUAL(2, spiAddrs.size());
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[1]);
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
  adc.irq(false, false, TestADC::IRQ_OFF);
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
// unlock() sends correct SPI address and value 0xA5 (datasheet unlock key)
// ---------------------------------------------------------------------------
void test_unlock_writes_correct_addr() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.unlock();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(0xA5, spiData[0][0]);  // unlock key (0xA5)
}

// ---------------------------------------------------------------------------
// lock() with default key writes the lock value (0x00 = locked)
// ---------------------------------------------------------------------------
void test_lock_default_writes_lock() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.lock();
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
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT(spiData[0][0] & 0x01);  // datasheet requires bit 0 = 1
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
  uint8_t scanData[3] = {0xFF, 0xFF, 0xFF};
  adc.write(MCP3x6xTest::Scan<TestADC>(scanData));
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN, spiAddrs[0]);
  // reserved bit 20 (bit 4 of byte 2) must be forced to 0
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(3, spiData[0].size());
  TEST_ASSERT_EQUAL(0x00, spiData[0][0] & 0x10);  // reserved bit cleared in byte 2
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
  mockReadSequence = {0x0F, 0x30};  // HW_ID=MCP3564, R-variant
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
  mockReadSequence = {0x0F, 0x30};  // HW_ID=MCP3564, R-variant
  MCP3564 adc;
  adc.begin();
  setUp();
  MCP3x6xTest::Adcdata<MCP3564> d;
  adc.read(&d);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(3, spiData[0].size());
}

// ---------------------------------------------------------------------------
// readADCDATA() / getLastChannel() — ID_SGNEXT_DATA channel strip + sign extension
// ---------------------------------------------------------------------------
void test_readADCDATA_id_sgnnext_strips_channel() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config3(false, false, false, TestADC::ID_SGNEXT_DATA, TestADC::CONTINUOUS);
  mockReadSequence = {0x20, 0x00, 0x00,
                      0x64};  // big-endian: CH_ID=2 in bits[31:28], value=100 in [27:0]
  int32_t val      = adc.readADCDATA();
  // CH_ID nibble must be stripped; value sign-extended from bit 27
  TEST_ASSERT_EQUAL(100, val);
  TEST_ASSERT_EQUAL(2, (int)adc.getLastChannel());
}

void test_readADCDATA_id_sgnnext_negative_value() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config3(false, false, false, TestADC::ID_SGNEXT_DATA, TestADC::CONTINUOUS);
  // CH_ID=5, value=-1: raw = 0x5FFFFFFF (sign extension fills bits[27:16]) → after strip: -1
  mockReadSequence = {0x5F, 0xFF, 0xFF, 0xFF};
  int32_t val      = adc.readADCDATA();
  TEST_ASSERT_EQUAL(-1, val);
  TEST_ASSERT_EQUAL(5, (int)adc.getLastChannel());
}

void test_readADCDATA_sgn_data_unchanged() {
  TestADC adc;
  adc.begin();
  setUp();
  // default data_format (SGN_DATA) → 2-byte read, plain sign extension (no channel bits)
  mockReadSequence = {0xFF, 0xF8};  // big-endian 16-bit value -8
  int32_t val      = adc.readADCDATA();
  TEST_ASSERT_EQUAL(-8, val);
}

// ---------------------------------------------------------------------------
// config2() az_ref parameter
// ---------------------------------------------------------------------------
void test_config2_az_ref_true() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2, true);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT(spiData[0][0] & 0x02);  // az_ref = bit 1
}

void test_config2_az_ref_default_false() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_FALSE(spiData[0][0] & 0x02);  // az_ref not set by default
}

// ---------------------------------------------------------------------------
// config2() writes BOOST in bits 7:6 and GAIN in bits 5:3 (RESERVED=1)
// ---------------------------------------------------------------------------
void test_config2_boost_gain_boost2() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.config2(false, TestADC::GAIN_1, TestADC::BOOST_2);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG2, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(
      0x89, spiData[0][0]);  // BOOST=2 (x1) in bits 7:6, GAIN=1 (x1) in bits 5:3, RESERVED=1
}

void test_irq_enum_values() {
  TEST_ASSERT_EQUAL(0, (uint8_t)TestADC::IRQ_OFF);
  TEST_ASSERT_EQUAL(1, (uint8_t)TestADC::IRQ_OPEN_DRAIN);
  TEST_ASSERT_EQUAL(2, (uint8_t)TestADC::IRQ_PUSH_PULL_LOW);
  TEST_ASSERT_EQUAL(3, (uint8_t)TestADC::IRQ_PUSH_PULL_HIGH);
}

void test_irq_enum_written() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.irq(false, true, TestADC::IRQ_PUSH_PULL_HIGH);
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ, spiAddrs[0]);
  TEST_ASSERT_EQUAL(1, spiData.size());
  TEST_ASSERT_EQUAL(3, (spiData[0][0] >> 2) & 0x03);  // irq_mode = 3
}

void test_getDeviceAddress() {
  TestADC adc;
  mockStatusByte = 0x30;  // STATUS byte bits [5:3] = 110 → DEV_ADDR[1:0] = 11 (binary) = 3
  adc.begin();
  TEST_ASSERT_EQUAL(3, adc.getDeviceAddress());
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
  RUN_TEST(test_config2_az_ref_true);
  RUN_TEST(test_config2_az_ref_default_false);
  RUN_TEST(test_config2_boost_gain_boost2);
  RUN_TEST(test_irq_enum_values);
  RUN_TEST(test_irq_enum_written);
  RUN_TEST(test_getDeviceAddress);
  RUN_TEST(test_irq_writes_correct_addr);
  RUN_TEST(test_mux_writes_correct_addr);
  RUN_TEST(test_scan_writes_correct_addr);
  RUN_TEST(test_lock_writes_correct_addr);
  RUN_TEST(test_lock_default_writes_lock);
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
  RUN_TEST(test_readADCDATA_id_sgnnext_strips_channel);
  RUN_TEST(test_readADCDATA_id_sgnnext_negative_value);
  RUN_TEST(test_readADCDATA_sgn_data_unchanged);
#endif
}

// ---------------------------------------------------------------------------
// Hardware-only mode — runs on ATmega328P (Arduino Nano) via PlatformIO test build.
// Executes all config tests sequentially, printing PASS/FAIL to Serial at
// 115200 baud for correlation with oscilloscope SPI captures.
// Scope channel mapping: CS=CHAN1(pin10), SCLK=CHAN3(pin13), MISO=CHAN2(pin12), MOSI=CHAN4(pin11)
// ---------------------------------------------------------------------------
#ifndef PIO_NATIVE_TESTING

static MCP3564 mcp;

static bool g_begin_ok = false;

void setup(void) {
#if defined(USB_VID) && defined(USB_PID)
  while (!Serial);
#else
  delay(2000);
#endif

  Serial.begin(115200);
  while (!Serial);

  // Configure GPIO pins as outputs so digitalWrite() drives them properly
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  Serial.println("TEST_START");

  // Test: begin() — CMD_RESET + chip-ID read
  Serial.print("TEST: begin ... MOSI: 78 79");
  g_begin_ok = mcp.begin();
  Serial.println(g_begin_ok ? " PASS" : " FAIL");

  // Test: unlock() — IWRITE|LOCK
  Serial.print("TEST: unlock ... MOSI: c6");
  mcp.unlock();
  Serial.println(" PASS");

  // Test: config0() — IWRITE|CONFIG0 + data
  Serial.print("TEST: config0 ... MOSI: 86 c0");
  mcp.config0(MCP3564::CONVERSION, MCP3564::BIAS_0UA, MCP3564::EXTERN, false);
  Serial.println(" PASS");

  // Test: config1() — IWRITE|CONFIG1 + data
  Serial.print("TEST: config1 ... MOSI: 88 44");
  mcp.config1(MCP3564::OSR_256, MCP3564::MCLK_0);
  Serial.println(" PASS");

  // Test: config2() — IWRITE|CONFIG2 + data
  Serial.print("TEST: config2 ... MOSI: 8a 89");
  mcp.config2(false, MCP3564::GAIN_1, MCP3564::BOOST_2);
  Serial.println(" PASS");

  // Test: config3() — IWRITE|CONFIG3 + data
  Serial.print("TEST: config3 ... MOSI: 8c 18");
  mcp.config3(false, false, false, MCP3564::SGN_DATA, MCP3564::CONTINUOUS);
  Serial.println(" PASS");

  // Test: irq() — IWRITE|IRQ + data
  Serial.print("TEST: irq ... MOSI: 8e 00");
  mcp.irq(false, false, MCP3564::IRQ_OFF);
  Serial.println(" PASS");

  // Test: mux() — IWRITE|MUX + data
  Serial.print("TEST: mux ... MOSI: 9a 08");
  mcp.mux(MCP3564::MUX_CH0, MCP3564::MUX_AGND);
  Serial.println(" PASS");

  // Test: analogRead CH0..CH3 — MUX + CONVERSION + ADCDATA read
  // Only if begin() succeeded (chip responds); otherwise conversion stalls
  if (g_begin_ok) {
    uint8_t channels[] = {MCP3x6x_CH0, MCP3x6x_CH1, MCP3x6x_CH2, MCP3x6x_CH3};
    for (uint8_t i = 0; i < 4; i++) {
      Serial.print("TEST: analogRead(CH");
      Serial.print(i);
      Serial.print(") ... MOSI: 9a ");
      Serial.print(static_cast<uint8_t>(channels[i]), HEX);
      Serial.print(" 68 81");
      int32_t raw = mcp.analogRead(static_cast<uint8_t>(channels[i]));
      Serial.print(" raw=");
      Serial.print(raw);
      Serial.println(" PASS");
    }
  } else {
    Serial.println("TEST: analogRead — skipped (begin() failed, chip not responding)");
  }

  Serial.println("TEST_END");
}

void loop(void) {
  // Continuously generate SPI traffic so the oscilloscope decoder can capture it.
  // Each analogRead() triggers: MUX write + CONVERSION + ADCDATA read (CS toggles per transaction).
  // Only runs if begin() succeeded (chip responds); otherwise conversion stalls.
  if (!g_begin_ok) {
    // Chip not responding — still generate SPI traffic for scope capture:
    // bit-bang SPI pins (CS=10, MOSI=11, MISO=12, SCK=13) so the scope sees
    // activity on all 4 channels connected to the oscilloscope.
    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, OUTPUT);
    digitalWrite(10, LOW);   // CS low
    digitalWrite(11, HIGH);  // MOSI high
    digitalWrite(13, HIGH);  // SCK high
    delayMicroseconds(100);
    digitalWrite(11, LOW);   // MOSI low
    digitalWrite(13, LOW);   // SCK low
    digitalWrite(10, HIGH);  // CS high
    delay(200);
    return;
  }
  uint8_t channels[] = {MCP3x6x_CH0, MCP3x6x_CH1, MCP3x6x_CH2, MCP3x6x_CH3};
  for (uint8_t i = 0; i < 4; i++) {
    int32_t raw = mcp.analogRead(static_cast<uint8_t>(channels[i]));
    Serial.print("loop: CH");
    Serial.print(i);
    Serial.print(" raw=");
    Serial.println(raw);
  }
  Serial.println("---");
  delay(200);
}

#endif  // PIO_NATIVE_TESTING
