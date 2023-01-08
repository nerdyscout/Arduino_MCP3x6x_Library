
#include "test_MCP3x6x.h"

#include <ArduinoFake.h>
#include <unity.h>

#include "MCP3x6x.h"

extern MCP3x6x mcp;

void test_settings_id(void) { TEST_ASSERT_EQUAL_UINT16(MCP3462_DEVICE_TYPE, mcp.settings.id); }

void test_settings_defaults(void) {
  mcp.reset(true);

  TEST_ASSERT_EQUAL(0xC0, mcp.settings.config0.raw);
  TEST_ASSERT_EQUAL(0x0C, mcp.settings.config1.raw);
  TEST_ASSERT_EQUAL(0x8B, mcp.settings.config2.raw);
  TEST_ASSERT_EQUAL(0x00, mcp.settings.config3.raw);
  TEST_ASSERT_EQUAL(0x73, mcp.settings.irq.raw);
  TEST_ASSERT_EQUAL(0x01, mcp.settings.mux.raw);
  //  TEST_ASSERT_EQUAL_INT_ARRAY({0x00, 0x00, 0x00}, mcp.settings.scan.raw, 3);
  //  TEST_ASSERT_EQUAL_INT_ARRAY({0x00, 0x00, 0x00}, mcp.settings.timer.raw, 3);
  //  TEST_ASSERT_EQUAL_INT_ARRAY({0x00, 0x00, 0x00}, mcp.settings.offsetcal.raw, 3);
  //  TEST_ASSERT_EQUAL_INT_ARRAY({0x80, 0x00, 0x00}, mcp.settings.gaincal.raw, 3);
  //  TEST_ASSERT_EQUAL_INT_ARRAY((0x90, 0x00, 0x00), mcp.settings.reserverd1, 3);
  //  TEST_ASSERT_EQUAL(0x50, mcp.settings.reserverd2);
  //  TEST_ASSERT_EQUAL(0xA5, mcp.settings.lock.raw);
  //  TEST_ASSERT_EQUAL_INT_ARRAY((0x00, 0x00), mcp.settings.crccfg.raw, 2);
}
