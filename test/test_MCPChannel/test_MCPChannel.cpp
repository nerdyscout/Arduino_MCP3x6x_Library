
#include <ArduinoFake.h>
#include <unity.h>

#include "MCPChannel.h"

using namespace fakeit;

void setUp(void) {}

void tearDown(void) {}

int runUnityTests(void) {
  UNITY_BEGIN();

  //  RUN_TEST(name);

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
