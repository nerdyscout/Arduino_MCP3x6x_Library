// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Stefan Herold

/**
 * @file test_MCP356x.cpp
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

#include "Arduino_MCP3x6x_Library.hpp"
#include "test_MCP356x.h"

void setup(void) {
  // Wait before the Unity test runner
  // establishes connection with a board Serial interface
#if defined(USB_VID) && defined(USB_PID)
  while (!Serial);
#else
  delay(2000);
#endif

  runUnityTests();
}

void loop(void) { delay(1000); }

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
  When(OverloadedMethod(ArduinoFake(SPI), beginTransaction, void(SPISettings)))
      .AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), endTransaction, void(void)))
      .AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), transfer, byte(uint8_t)))
      .AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), transfer, void(void *, size_t)))
      .AlwaysReturn();
#endif
}

void suiteTearDown(void) {}

void setUp(void) {}

void tearDown(void) {}

// actual test cases

void test_instance(void) {
  MCP3561 mcp1();
  MCP3562 mcp2();
  MCP3564 mcp4();
}
