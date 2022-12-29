#include <unity.h>

#include "MCP3x6x.h"

MCP3464 mcp;

void mcp_wrapper() { mcp.IRQ_handler(); }

void setUp(void) { mcp.begin(); }

void tearDown(void) { mcp.reset(); }

void test_ResolutionMax(void) { TEST_ASSERT_EQUAL(16, mcp.getResolution()); }

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_ResolutionMax);
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
