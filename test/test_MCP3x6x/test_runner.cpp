// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// SPI tracking definitions (native mock mode only)
// ---------------------------------------------------------------------------
#ifdef PIO_NATIVE_TESTING
std::vector<uint8_t> spiAddrs;
std::vector<std::vector<uint8_t>> spiData;
std::vector<uint8_t> mockReadSequence;
#endif
uint8_t mockStatusByte = 0x00;
uint8_t lastSpiAddress = 0x00;

bool digitalWriteCalled   = false;
uint8_t digitalWritePin   = 0xFF;
uint8_t digitalWriteValue = 0xFF;

bool pinModeCalled        = false;
uint8_t pinModePin        = 0xFF;
uint8_t pinModeMode       = 0xFF;

// ---------------------------------------------------------------------------
// suiteSetUp / setUp / tearDown
// ---------------------------------------------------------------------------
void suiteSetUp(void) {
#ifdef PIO_NATIVE_TESTING
  ArduinoFakeReset();

  // Mock global Arduino functions
  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
  When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
  When(Method(ArduinoFake(), delay)).AlwaysReturn();
  When(Method(ArduinoFake(), millis)).AlwaysReturn(0);
  When(Method(ArduinoFake(), attachInterrupt)).AlwaysReturn();
  When(Method(ArduinoFake(), detachInterrupt)).AlwaysReturn();
  When(Method(ArduinoFake(), noTone)).AlwaysReturn();
  When(Method(ArduinoFake(), sei)).AlwaysReturn();
  When(Method(ArduinoFake(), cli)).AlwaysReturn();

  // SPI — use OverloadedMethod for all overloaded functions
  When(OverloadedMethod(ArduinoFake(SPI), begin, void(void))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), end, void(void))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), beginTransaction, void(SPISettings))).AlwaysReturn();
  When(OverloadedMethod(ArduinoFake(SPI), endTransaction, void(void))).AlwaysReturn();

  When(OverloadedMethod(ArduinoFake(SPI), transfer, byte(uint8_t)))
      .AlwaysDo([](uint8_t addr) -> byte {
        lastSpiAddress = addr;
        spiAddrs.push_back(addr);
        return mockStatusByte;
      });

  When(OverloadedMethod(ArduinoFake(SPI), transfer, void(void*, size_t)))
      .AlwaysDo([](void* data, size_t size) {
        std::vector<uint8_t> bytes(size);
        std::memcpy(bytes.data(), data, size);
        spiData.push_back(bytes);

        if ((lastSpiAddress & 0x01) && size > 0) {
          if (!mockReadSequence.empty()) {
            size_t n = size < mockReadSequence.size() ? size : mockReadSequence.size();
            std::memcpy(data, mockReadSequence.data(), n);
            mockReadSequence.erase(mockReadSequence.begin(), mockReadSequence.begin() + n);
          } else {
            uint8_t reg = (lastSpiAddress >> 2) & 0x0F;
            std::memset(data, 0x00, size);
            if (reg == 0x0E) {
              ((uint8_t*)data)[0] = 0x08;  // MCP3461 HW_ID (TestADC default)
            } else if (reg == 0x0C) {
              ((uint8_t*)data)[0] = 0x30;  // R-variant
            }
          }
        }
      });
#endif
}

int suiteTearDown(int num_failures) { return num_failures; }

void setUp(void) {
#ifdef PIO_NATIVE_TESTING
  spiAddrs.clear();
  spiData.clear();
  mockReadSequence.clear();
#endif
  digitalWriteCalled = false;
  digitalWritePin    = 0xFF;
  digitalWriteValue  = 0xFF;
  pinModeCalled      = false;
  pinModePin         = 0xFF;
  pinModeMode        = 0xFF;
}

void tearDown(void) {}

// ---------------------------------------------------------------------------
// HIL smoke test — mock mode
// ---------------------------------------------------------------------------

#ifdef PIO_NATIVE_TESTING

void test_hil_smoke_begin() {
  MCP3564 mcp;
  mockStatusByte   = 0x01;   // POR=1 → reset() succeeds
  mockReadSequence = {0x0F, 0x30};  // HW_ID=0xF (MCP3564), R-variant=0x30
  bool ok = mcp.begin();
  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_FALSE(spiAddrs.empty());
}

void test_hil_smoke_register_write() {
  MCP3564 mcp;
  mockStatusByte   = 0x01;
  mockReadSequence = {0x0F, 0x30};
  mcp.begin();
  setUp();

  mcp.unlock();
  // Command byte = SPI_ADR(0x40) | LOCK_REG(0x0D<<2=0x34) | IWRITE_type(0x02) = 0x76
  TEST_ASSERT_EQUAL_HEX8(0x76, spiAddrs[0]);
  TEST_ASSERT_EQUAL(0xA5, spiData[0][0]);  // unlock password

  mcp.setReference(2.4f);  // CONFIG0.VREF_SEL=internal
  // Command byte = SPI_ADR(0x40) | CONFIG0_REG(0x01<<2=0x04) | IWRITE_type(0x02) = 0x46
  TEST_ASSERT_EQUAL_HEX8(0x46, spiAddrs[spiAddrs.size() - 1]);
}

void test_hil_smoke_status_bits() {
  MCP3564 mcp;
  mockStatusByte   = 0x01;
  mockReadSequence = {0x0F, 0x30};
  mcp.begin();
  setUp();

  mockStatusByte = 0x04;  // DR=1 → status_dr() = true (non-inverted)
  mcp.reset();  // SPI transaction updates _status from mockStatusByte
  TEST_ASSERT_TRUE(mcp.status_dr());
  mockStatusByte = 0x00;  // DR=0 → status_dr() = false
  mcp.reset();  // SPI transaction updates _status from mockStatusByte
  TEST_ASSERT_FALSE(mcp.status_dr());
}

// ---------------------------------------------------------------------------
// Chip-ID diagnostic — mock mode
// ---------------------------------------------------------------------------

struct ChipIDCase {
  const char*  name;
  uint8_t      hw_id_nibble;   // value injected by mock SPI
  bool         expect_begin_ok;
};

static const ChipIDCase chip_id_cases[] = {
  {"MCP3461 (16-bit, 2ch)",  0x8, true},
  {"MCP3462 (16-bit, 4ch)",   0x9, true},
  {"MCP3464 (16-bit, 8ch)",   0xB, true},
  {"MCP3561 (24-bit, 2ch)",   0xC, true},
  {"MCP3562 (24-bit, 4ch)",   0xD, true},
  {"MCP3564 (24-bit, 8ch)",   0xF, true},
  {"Unknown (0x0)",           0x0, false},
  {"Unknown (0x1)",           0x1, false},
  {"Unknown (0x2)",           0x2, false},
  {"Unknown (0x3)",           0x3, false},
  {"Unknown (0x4)",           0x4, false},
  {"Unknown (0x5)",           0x5, false},
  {"Unknown (0x6)",           0x6, false},
  {"Unknown (0x7)",           0x7, false},
  {"Unknown (0xA)",           0xA, false},
  {"Unknown (0xE)",           0xE, false},
};

void test_chipid_all_variants() {
  for (size_t i = 0; i < sizeof(chip_id_cases) / sizeof(chip_id_cases[0]); i++) {
    const ChipIDCase& c = chip_id_cases[i];

    MCP3564 mcp;
    mockStatusByte   = 0x01;   // POR=1 → reset() succeeds
    mockReadSequence = {c.hw_id_nibble, 0x30};  // HW_ID nibble + R-variant
    bool ok = mcp.begin();

    if (c.expect_begin_ok && c.hw_id_nibble == 0xF) {
      TEST_ASSERT_TRUE_MESSAGE(ok, c.name);
    } else if (!c.expect_begin_ok) {
      TEST_ASSERT_FALSE_MESSAGE(ok, c.name);
    }

    setUp();
  }
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

void register_hil_smoke_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_hil_smoke_begin);
  RUN_TEST(test_hil_smoke_register_write);
  RUN_TEST(test_hil_smoke_status_bits);
#endif
}

void register_chipid_diag_tests() {
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_chipid_all_variants);
#endif
}

void register_scope_tests() {
#if defined(PIO_NATIVE_TESTING)
  // scope tests (test_scope_*) are not implemented; registration intentionally empty
#endif
}

// ---------------------------------------------------------------------------
// Run all tests
// ---------------------------------------------------------------------------
int runUnityTests(void) {
  UNITY_BEGIN();
  register_basic_tests();

#ifdef PIO_NATIVE_TESTING
  register_config_tests();
  register_analog_tests();
  register_scan_mode_tests();
  register_fastcmd_status_tests();
  register_misc_tests();
  register_dma_tests();

  RUN_TEST(test_hil_smoke_begin);
  RUN_TEST(test_hil_smoke_register_write);
  RUN_TEST(test_hil_smoke_status_bits);
  RUN_TEST(test_chipid_all_variants);
#endif

  return UNITY_END();
}

#endif  // PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// HIL smoke test — hardware-only mode
// ---------------------------------------------------------------------------

#ifndef PIO_NATIVE_TESTING

static MCP3564 mcp;

static void printStatus(const char* where) {
  Serial.print(where);
  Serial.print(": DR=");
  Serial.print(mcp.status_dr());
  Serial.print(" CRCCFG=");
  Serial.print(mcp.status_crccfg());
  Serial.print(" POR=");
  Serial.print(mcp.status_por());
  Serial.print(" ADDR=");
  Serial.println(mcp.getDeviceAddress());
}

void setup(void) {
#if defined(USB_VID) && defined(USB_PID)
  while (!Serial);
#else
  delay(2000);
#endif
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  UNITY_BEGIN();
  register_basic_tests();

#ifdef PIO_NATIVE_TESTING
  register_config_tests();
  register_analog_tests();
  register_scan_mode_tests();
  register_fastcmd_status_tests();
  register_misc_tests();
  register_dma_tests();

  RUN_TEST(test_hil_smoke_begin);
  RUN_TEST(test_hil_smoke_register_write);
  RUN_TEST(test_hil_smoke_status_bits);
  RUN_TEST(test_chipid_all_variants);
#endif

  UNITY_END();
}

void loop(void) {
  printStatus("status");
  delay(1000);
}

#endif  // PIO_NATIVE_TESTING

#ifdef PIO_NATIVE_TESTING
int main(int argc, char** argv) {
  suiteSetUp();
  int result = runUnityTests();
  return suiteTearDown(result);
}
#endif
