// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#ifndef TEST_INIT_H_
#define TEST_INIT_H_

#ifdef PIO_NATIVE_TESTING
  #include <ArduinoFake.h>
// cppcheck-suppress syntaxError
using namespace fakeit;
  #include <unity.h>
  #include <cstring>
  #include <vector>

  // ---------------------------------------------------------------------------
  // SPI tracking (native mock mode only)
  // ---------------------------------------------------------------------------
  extern std::vector<uint8_t> spiAddrs;
  extern std::vector<std::vector<uint8_t>> spiData;
  extern uint8_t mockStatusByte;
  extern uint8_t lastSpiAddress;
  extern std::vector<uint8_t> mockReadSequence;

  extern bool digitalWriteCalled;
  extern uint8_t digitalWritePin;
  extern uint8_t digitalWriteValue;

  extern bool pinModeCalled;
  extern uint8_t pinModePin;
  extern uint8_t pinModeMode;
#else
  #include <Arduino.h>
  #include <SPI.h>
  #include <unity.h>
#endif

#ifdef PIO_NATIVE_TESTING
  #include <cstring>
#endif

#include "Arduino_MCP3x6x_Library.hpp"

// ---------------------------------------------------------------------------
// ADC variant selection
// ---------------------------------------------------------------------------
#ifdef PIO_NATIVE_TESTING
  #define FOR_EACH_ADC(M) M(MCP3461) M(MCP3462) M(MCP3464) M(MCP3561) M(MCP3562) M(MCP3564)
using TestADC = MCP3461;
#else
  #ifndef TEST_ADC_TYPE
    #error "Define TEST_ADC_TYPE for hardware testing (e.g. -DTEST_ADC_TYPE=MCP3564)"
  #endif
  #define FOR_EACH_ADC(M) M(TEST_ADC_TYPE)
using TestADC = TEST_ADC_TYPE;
#endif

// ---------------------------------------------------------------------------
// Singleton ADC instance helpers (reused across tests)
// ---------------------------------------------------------------------------
template <typename T>
T* createADC() {
  return new T();
}

// ---------------------------------------------------------------------------
// Friend struct for testing private members (native only)
// ---------------------------------------------------------------------------
#ifdef PIO_NATIVE_TESTING
struct MCP3x6xTest {
  template <typename T>
  static int32_t getValue(T& adc, int32_t raw) {
    return adc._getValue(raw);
  }

  template <typename T>
  static int8_t getChannel(T& adc, uint32_t raw) {
    return adc._getChannel(raw);
  }

  template <typename T>
  static void dmaComplete(T& adc, uint32_t timestamp) {
    adc._dmaComplete(timestamp);
  }

  template <typename T>
  static typename T::status_t fastcmd(T& adc, uint8_t cmd) {
    return adc._fastcmd(cmd);
  }

  template <typename T>
  using Adcdata = typename T::Adcdata;

  template <typename T>
  using Config0 = typename T::Config0;

  template <typename T>
  using Config1 = typename T::Config1;

  template <typename T>
  using Config2 = typename T::Config2;

  template <typename T>
  using Config3 = typename T::Config3;

  template <typename T>
  using Irq = typename T::Irq;

  template <typename T>
  using Mux = typename T::Mux;

  template <typename T>
  using Scan = typename T::Scan;

  template <typename T>
  using Lock = typename T::Lock;
};
#endif

// ---------------------------------------------------------------------------
// Template test helpers
// ---------------------------------------------------------------------------
template <typename T>
void test_instance() {
  T* p = new T();
  TEST_ASSERT_NOT_NULL(p);
  delete p;
}

template <typename T>
void test_begin() {
  T adc;
  bool ok = adc.begin();
  TEST_ASSERT_TRUE(ok);
}

// ---------------------------------------------------------------------------
// Registration function declarations (defined in tests.hpp)
// ---------------------------------------------------------------------------
void register_init_tests();
void register_config_tests();
void register_conversion_tests();
void register_status_tests();
void register_dma_tests();
void register_calibration_tests();
void register_spi_tests();

// ---------------------------------------------------------------------------
// Uniform entry point
// ---------------------------------------------------------------------------
int runUnityTests(void);

#endif  // TEST_INIT_H_
