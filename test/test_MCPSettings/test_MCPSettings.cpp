#include "test_MCPSettings.h"

#include <unity.h>

#include "MCPSettings.hpp"

MCPSettings settings;

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

void setUp(void) {}

void tearDown(void) {}

int runUnityTests(void) {
  UNITY_BEGIN();

  RUN_TEST(test_settings_defaults);

  return UNITY_END();
}

void test_settings_defaults(void) {
  TEST_ASSERT_EQUAL(0xC0, settings.config0.raw);
  TEST_ASSERT_EQUAL(0x0C, settings.config1.raw);
  TEST_ASSERT_EQUAL(0x8B, settings.config2.raw);
  TEST_ASSERT_EQUAL(0x00, settings.config3.raw);
  TEST_ASSERT_EQUAL(0x73, settings.irq.raw);
  TEST_ASSERT_EQUAL(0x01, settings.mux.raw);
  //  TEST_ASSERT_EQUAL_INT_ARRAY({0x00, 0x00, 0x00}, mcp.settings.scan.raw, 3);
  //  TEST_ASSERT_EQUAL_INT_ARRAY({0x00, 0x00, 0x00}, mcp.settings.timer.raw, 3);
  //  TEST_ASSERT_EQUAL_INT_ARRAY({0x00, 0x00, 0x00}, mcp.settings.offsetcal.raw, 3);
  //  TEST_ASSERT_EQUAL_INT_ARRAY({0x80, 0x00, 0x00}, mcp.settings.gaincal.raw, 3);
  //  TEST_ASSERT_EQUAL_INT_ARRAY((0x90, 0x00, 0x00), mcp.settings.reserverd1, 3);
  //  TEST_ASSERT_EQUAL(0x50, mcp.settings.reserverd2);
  //  TEST_ASSERT_EQUAL(0xA5, mcp.settings.lock.raw);
  //  TEST_ASSERT_EQUAL_INT_ARRAY((0x00, 0x00), mcp.settings.crccfg.raw, 2);
}
