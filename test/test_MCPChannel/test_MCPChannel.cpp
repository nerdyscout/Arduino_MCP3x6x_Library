#include "test_MCPChannel.h"

#include <unity.h>

#include "MCPChannel.hpp"

#ifdef PIO_UNIT_TESTING
#  include <ArduinoFake.h>
using namespace fakeit;

/**
 * For native dev-platform or for some embedded frameworks
 */
int main(void) { return runUnityTests(); }

#else

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

  //  RUN_TEST(name);

  return UNITY_END();
}
