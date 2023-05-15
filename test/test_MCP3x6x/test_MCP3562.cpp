#include "test_MCP3562.h"

#include <unity.h>

#include "MCP3x6x.hpp"

MCP3562 mcp;

#ifdef PIO_UNIT_TESTING
#  include <ArduinoFake.h>

using namespace fakeit;

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) { return runUnityTests(); }

#else
#  include <Arduino.h>

/**
 * For Arduino framework
 */
void setup(void) {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop(void) {}

#endif

void setUp(void) {
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

  //  mcp.begin();
}

void tearDown(void) { mcp.reset(); }

int runUnityTests(void) {
  UNITY_BEGIN();

  //  RUN_TEST(name);

  return UNITY_END();
}
