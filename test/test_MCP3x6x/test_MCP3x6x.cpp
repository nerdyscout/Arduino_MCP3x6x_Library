// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

/**
 * @file test_MCP3x6x.cpp
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief Unified tests for all MCP3x6x variants
 * @version 0.1.0
 * @date 2025-10-14
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifdef PIO_NATIVE_TESTING
  #include <ArduinoFake.h>
#else
  #include <Arduino.h>
#endif

#include <unity.h>

#include <algorithm>
#include <cstring>
#include <vector>

#include "Arduino_MCP3x6x_Library.hpp"

using namespace fakeit;

// SPI transfer tracking
static std::vector<uint8_t> spiAddrs;
static std::vector<std::vector<uint8_t>> spiData;

// --- SPI address constants ---
constexpr uint8_t ADR_IRQ_WRITE   = MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_IRQ;
constexpr uint8_t ADR_MUX_WRITE   = MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_MUX;
constexpr uint8_t ADR_SCAN_WRITE  = MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_SCAN;
constexpr uint8_t ADR_LOCK_WRITE  = MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK;
constexpr uint8_t ADR_CONFIG0     = MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0;
constexpr uint8_t ADR_ADCDATA_READ = MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA;
constexpr uint8_t ADR_CONVERSION  = MCP3x6x_CMD_CONVERSION;

// --- Generic test templates ---
template <typename T>
void test_instance() {
  T* p = nullptr;
  p    = new T();

  TEST_ASSERT_TRUE_MESSAGE(p, "Instance not created");
  delete p;
}

template <typename T>
void test_begin() {
  T adc;
  bool ok = adc.begin();

  TEST_ASSERT_TRUE_MESSAGE(ok, "begin() failed");
}

// --- Variant registration ---
#define MCP_VARIANTS(V) \
  V(MCP3461)            \
  V(MCP3462)            \
  V(MCP3464)            \
  V(MCP3561)            \
  V(MCP3562)            \
  V(MCP3564)

#define TEST_INSTANCE(V) \
  void test_instance_##V() { test_instance<V>(); }
#define TEST_BEGIN(V) \
  void test_begin_##V() { test_begin<V>(); }

MCP_VARIANTS(TEST_INSTANCE)
MCP_VARIANTS(TEST_BEGIN)

// --- Bug regression tests ---

void test_configure_writes_irq_when_changed(void) {
  MCP3561 adc;
  spiAddrs.clear();
  spiData.clear();

  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2,
                MCP3x6x_CFG_CONFIG3, (uint8_t)0x00, MCP3x6x_CFG_MUX,
                MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER, MCP3x6x_CFG_OFFSET,
                MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);

  bool found = std::find(spiAddrs.begin(), spiAddrs.end(), ADR_IRQ_WRITE) != spiAddrs.end();
  TEST_ASSERT_TRUE_MESSAGE(found, "IRQ not written when value changed");
}

void test_configure_writes_mux_when_changed(void) {
  MCP3561 adc;
  spiAddrs.clear();
  spiData.clear();

  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2,
                MCP3x6x_CFG_CONFIG3, MCP3x6x_CFG_IRQ, (uint8_t)0x00,
                MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER, MCP3x6x_CFG_OFFSET,
                MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);

  bool found = std::find(spiAddrs.begin(), spiAddrs.end(), ADR_MUX_WRITE) != spiAddrs.end();
  TEST_ASSERT_TRUE_MESSAGE(found, "MUX not written when value changed");
}

void test_configure_writes_lock_when_changed(void) {
  MCP3561 adc;
  spiAddrs.clear();
  spiData.clear();

  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2,
                MCP3x6x_CFG_CONFIG3, MCP3x6x_CFG_IRQ, MCP3x6x_CFG_MUX,
                MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER, MCP3x6x_CFG_OFFSET,
                MCP3x6x_CFG_GAIN, (uint8_t)0x00, MCP3x6x_CFG_CRCCFG);

  bool found = std::find(spiAddrs.begin(), spiAddrs.end(), ADR_LOCK_WRITE) != spiAddrs.end();
  TEST_ASSERT_TRUE_MESSAGE(found, "LOCK not written when value changed");
}

void test_configure_skips_unchanged(void) {
  MCP3561 adc;
  spiAddrs.clear();
  spiData.clear();

  // call with all defaults — no register writes expected
  adc.configure(MCP3x6x_CFG_CONFIG0, MCP3x6x_CFG_CONFIG1, MCP3x6x_CFG_CONFIG2,
                MCP3x6x_CFG_CONFIG3, MCP3x6x_CFG_IRQ, MCP3x6x_CFG_MUX,
                MCP3x6x_CFG_SCAN, MCP3x6x_CFG_TIMER, MCP3x6x_CFG_OFFSET,
                MCP3x6x_CFG_GAIN, MCP3x6x_CFG_LOCK, MCP3x6x_CFG_CRCCFG);

  bool irqFound =
      std::find(spiAddrs.begin(), spiAddrs.end(), ADR_IRQ_WRITE) != spiAddrs.end();
  bool muxFound =
      std::find(spiAddrs.begin(), spiAddrs.end(), ADR_MUX_WRITE) != spiAddrs.end();
  bool lockFound =
      std::find(spiAddrs.begin(), spiAddrs.end(), ADR_LOCK_WRITE) != spiAddrs.end();
  TEST_ASSERT_FALSE_MESSAGE(irqFound, "IRQ written despite matching default");
  TEST_ASSERT_FALSE_MESSAGE(muxFound, "MUX written despite matching default");
  TEST_ASSERT_FALSE_MESSAGE(lockFound, "LOCK written despite matching default");
}

void test_enable_scan_channel_sets_bit(void) {
  MCP3561 adc;
  spiAddrs.clear();
  spiData.clear();

  adc.enableScanChannel(MCP3x6x_CH0);

  // verify SCAN register was written with bit 0 set
  for (size_t i = 0; i < spiAddrs.size(); i++) {
    if (spiAddrs[i] == ADR_SCAN_WRITE) {
      // SCAN register: raw[0] contains channel bits (little-endian)
      TEST_ASSERT_TRUE_MESSAGE(spiData[i].size() == 3,
                               "SCAN write should transfer 3 bytes");
      TEST_ASSERT_TRUE_MESSAGE(spiData[i][0] & 0x01,
                               "SCAN bit 0 (CH0) was not set");
      return;
    }
  }
  TEST_FAIL_MESSAGE("SCAN register was not written");
}

void test_disable_scan_channel_clears_bit(void) {
  MCP3561 adc;
  spiAddrs.clear();
  spiData.clear();

  adc.enableScanChannel(MCP3x6x_CH0);
  // reset tracking for the disable call
  spiAddrs.clear();
  spiData.clear();

  adc.disableScanChannel(MCP3x6x_CH0);

  // after disable, bit 0 should be 0
  for (size_t i = 0; i < spiAddrs.size(); i++) {
    if (spiAddrs[i] == ADR_SCAN_WRITE) {
      TEST_ASSERT_TRUE_MESSAGE(spiData[i].size() == 3,
                               "SCAN write should transfer 3 bytes");
      TEST_ASSERT_TRUE_MESSAGE(!(spiData[i][0] & 0x01),
                               "SCAN bit 0 (CH0) was not cleared");
      return;
    }
  }
  TEST_FAIL_MESSAGE("SCAN register was not written");
}

void test_analogRead_differential_waits_for_dr(void) {
  MCP3561 adc;

  // override SPI mock: first 3 byte-transfers return DR=0, rest return DR=1
  When(OverloadedMethod(ArduinoFake(SPI), transfer, byte(uint8_t)))
      .AlwaysDo([](uint8_t addr) -> byte {
        static int n = 0;
        spiAddrs.push_back(addr);
        n++;
        return (n > 3) ? 0x05 : 0x00;  // DR=1 after 3rd call
      });

  spiAddrs.clear();
  adc.analogReadDifferential(MCP3x6x_CH0, MCP3x6x_CH1);

  // verify multiple ADCDATA reads occurred (indicates busy-wait loop)
  int adcdataReadCount = 0;
  for (auto addr : spiAddrs) {
    if (addr == ADR_ADCDATA_READ) adcdataReadCount++;
  }
  TEST_ASSERT_TRUE_MESSAGE(adcdataReadCount >= 2,
                           "analogReadDifferential should busy-wait and read "
                           "ADCDATA multiple times");
}

// --- Unity test registration ---

int runUnityTests(void) {
  UNITY_BEGIN();

  // Variant instance tests
  RUN_TEST(test_instance_MCP3461);
  RUN_TEST(test_instance_MCP3462);
  RUN_TEST(test_instance_MCP3464);
  RUN_TEST(test_instance_MCP3561);
  RUN_TEST(test_instance_MCP3562);
  RUN_TEST(test_instance_MCP3564);

  // Variant begin tests
  RUN_TEST(test_begin_MCP3461);
  RUN_TEST(test_begin_MCP3462);
  RUN_TEST(test_begin_MCP3464);
  RUN_TEST(test_begin_MCP3561);
  RUN_TEST(test_begin_MCP3562);
  RUN_TEST(test_begin_MCP3564);

  // Bug regression tests
  RUN_TEST(test_configure_writes_irq_when_changed);
  RUN_TEST(test_configure_writes_mux_when_changed);
  RUN_TEST(test_configure_writes_lock_when_changed);
  RUN_TEST(test_configure_skips_unchanged);
  RUN_TEST(test_enable_scan_channel_sets_bit);
  RUN_TEST(test_disable_scan_channel_clears_bit);
  RUN_TEST(test_analogRead_differential_waits_for_dr);

  return UNITY_END();
}

// --- Entry points ---

#ifdef PIO_NATIVE_TESTING
int main(int argc, char** argv) {
  suiteSetUp();
  int result = runUnityTests();
  return suiteTearDown(result);
}
#else
void loop(void) {}

void setup(void) {
  #if defined(USB_VID) && defined(USB_PID)
    while (!Serial);
  #else
    delay(2000);
  #endif
  runUnityTests();
}
#endif

// --- FakeIt setup ---

void suiteSetUp(void) {
#ifdef PIO_NATIVE_TESTING
  ArduinoFakeReset();
  When(Method(ArduinoFake(), sei)).AlwaysReturn();
  When(Method(ArduinoFake(), cli)).AlwaysReturn();
  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
  When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
  When(Method(ArduinoFake(), attachInterrupt)).AlwaysReturn();
  When(Method(ArduinoFake(), detachInterrupt)).AlwaysReturn();
  When(Method(ArduinoFake(), noTone)).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), begin, void(void))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), end, void(void))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), beginTransaction, void(SPISettings)))
      .AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), endTransaction, void(void)))
      .AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), transfer, void(void*, size_t)))
      .AlwaysDo([](void* data, size_t size) {
        if (!spiData.empty()) {
          auto* bytes = static_cast<uint8_t*>(data);
          spiData.back().assign(bytes, bytes + size);
        }
      });
  // default SPI byte transfer returns DR=1, POR=1
  When(OverloadedMethod(ArduinoFake(SPI), transfer, byte(uint8_t)))
      .AlwaysDo([](uint8_t addr) -> byte {
        spiAddrs.push_back(addr);
        spiData.push_back({});
        return 0x05;
      });
#endif
}

int suiteTearDown(int num_failures) { return num_failures; }

void setUp(void) {
  spiAddrs.clear();
  spiData.clear();
}

void tearDown(void) {}
