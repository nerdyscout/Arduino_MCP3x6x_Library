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

#include "Arduino_MCP3x6x_Library.hpp"

// --- Generic test templates ---
template <typename T>
void test_instance() {
  T* p = nullptr;  // no instance
  p    = new T();  // instance created

  TEST_ASSERT_TRUE_MESSAGE(p, "Instance not created");
}

template <typename T>
void test_begin() {
  T adc;
  byte s = adc.status();

  TEST_ASSERT_TRUE_MESSAGE(s, String(s).c_str());
}

// --- Variant registration ---
#define MCP_VARIANTS(V) \
  V(MCP3461)            \
  V(MCP3462)            \
  V(MCP3464)            \
  V(MCP3561)            \
  V(MCP3562)            \
  V(MCP3564)

// Generate test functions for each variant
#define TEST_INSTANCE(V) \
  void test_instance_##V() { test_instance<V>(); }
#define TEST_BEGIN(V) \
  void test_begin_##V() { test_begin<V>(); }

MCP_VARIANTS(TEST_INSTANCE)
MCP_VARIANTS(TEST_BEGIN)

// --- Automated test registration ---
typedef void (*UnityTestFunc)(void);

// --- Unity setup ---

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_instance_MCP3461);
  RUN_TEST(test_instance_MCP3462);
  RUN_TEST(test_instance_MCP3464);
  RUN_TEST(test_instance_MCP3561);
  RUN_TEST(test_instance_MCP3562);
  RUN_TEST(test_instance_MCP3564);

  RUN_TEST(test_begin_MCP3461);
  RUN_TEST(test_begin_MCP3462);
  RUN_TEST(test_begin_MCP3464);
  RUN_TEST(test_begin_MCP3561);
  RUN_TEST(test_begin_MCP3562);
  RUN_TEST(test_begin_MCP3564);
  return UNITY_END();
}

void loop(void) {}

void setup(void) {
#if defined(USB_VID) && defined(USB_PID)
  while (!Serial);
#else
  delay(2000);
#endif
  runUnityTests();
}

void suiteSetUp(void) {
#ifdef PIO_NATIVE_TESTING
  using namespace fakeit;
  ArduinoFakeReset();
  When(Method(ArduinoFake(), sei)).AlwaysReturn();
  When(Method(ArduinoFake(), cli)).AlwaysReturn();
  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
  When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
  When(Method(ArduinoFake(), attachInterrupt)).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), begin, void(void))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), end, void(void))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), beginTransaction, void(SPISettings))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), endTransaction, void(void))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), transfer, byte(uint8_t))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), transfer, void(void*, size_t))).AlwaysReturn();
#endif
}

void suiteTearDown(void) {}

void setUp(void) {}

void tearDown(void) {}
