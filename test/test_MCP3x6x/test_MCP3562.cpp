#include <ArduinoFake.h>
#include <unity.h>

#include "MCP3x6x.h"

using namespace fakeit;

MCP3562 mcp;

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

  return UNITY_END();
}

#ifdef PIO_UNIT_TESTING

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) { return runUnityTests(); }

#else

/**
 * For Arduino framework
 */
void setup() {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  runUnityTests();
}

void loop() {}

#endif
