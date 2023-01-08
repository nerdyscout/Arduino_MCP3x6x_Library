#include <ArduinoFake.h>
#include <unity.h>

#include "../test_MCP3x6x.h"
#include "MCP3x6x.h"

using namespace fakeit;

MCP3462 mcp;

void mcp_wrapper() { mcp.IRQ_handler(); }

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

void test_Resolution(void) {
  TEST_ASSERT_EQUAL(16, mcp.getResolution());

  mcp.setDataFormat(MCP3x6x::data_format::SGN_DATA);
  TEST_ASSERT_EQUAL(15, mcp.getResolution());
}
// void test_ValueMax(void) { TEST_ASSERT_EQUAL(pow(2,16), mcp.getMaxValue()); }

int runUnityTests(void) {
  UNITY_BEGIN();

  // settings
  RUN_TEST(test_settings_id);
  RUN_TEST(test_settings_defaults);

  // resolution
  RUN_TEST(test_Resolution);

  return UNITY_END();
}

#ifdef UNIT_TEST_NATIVE

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
