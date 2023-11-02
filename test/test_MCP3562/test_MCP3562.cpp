// SPDX-License-Identifier: MIT

/**
 * @file test_MCP3562.cpp
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.2
 * @date 2023-10-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifdef PIO_NATIVE_TESTING
#  include <ArduinoFake.h>
#else
#  include <Arduino.h>
#endif

#include <unity.h>

#include "MCP3x6x.hpp"
#include "test_MCP3562.h"

void setup(void) {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop(void) {}

int runUnityTests(void) {
  UNITY_BEGIN();

  RUN_TEST(test_instance);

  return UNITY_END();
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
  When(OverloadedMethod(ArduinoFake(SPI), transfer, void(void *, size_t))).AlwaysReturn();
#endif
}

void suiteTearDown(void) {}

void setUp(void) {}

void tearDown(void) {}

// actual test cases

void test_instance(void) { MCP3562 mcp(); }
