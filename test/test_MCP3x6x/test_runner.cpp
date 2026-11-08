// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// SPI tracking definitions
// ---------------------------------------------------------------------------
std::vector<uint8_t> spiAddrs;
std::vector<std::vector<uint8_t>> spiData;
uint8_t mockStatusByte = 0x00;
uint8_t lastSpiAddress = 0x00;
std::vector<uint8_t> mockReadSequence;

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
  spiAddrs.clear();
  spiData.clear();
  mockReadSequence.clear();
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
  TEST_ASSERT_EQUAL_HEX8(0x0F, mcp.getDeviceAddress() & 0x0F);
  TEST_ASSERT_FALSE(spiAddrs.empty());
}

void test_hil_smoke_register_write() {
  MCP3564 mcp;
  mockStatusByte   = 0x01;
  mockReadSequence = {0x0F, 0x30};
  mcp.begin();
  setUp();

  mcp.unlock();
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_LOCK, spiAddrs[0]);
  TEST_ASSERT_EQUAL(0xA5, spiData[0][0]);  // unlock password

  mcp.setReference(2.4f);  // CONFIG0.VREF_SEL=internal
  TEST_ASSERT_EQUAL(MCP3x6x_CMD_IWRITE | MCP3x6x_ADR_CONFIG0, spiAddrs[spiAddrs.size() - 2]);
}

void test_hil_smoke_status_bits() {
  MCP3564 mcp;
  mockStatusByte   = 0x01;
  mockReadSequence = {0x0F, 0x30};
  mcp.begin();
  setUp();

  mockStatusByte = 0x04;  // DR=1
  TEST_ASSERT_FALSE(mcp.status_dr());  // DR inverted
  mockStatusByte = 0x00;
  TEST_ASSERT_TRUE(mcp.status_dr());
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

void setup() {
#if defined(USB_VID) && defined(USB_PID)
  while (!Serial);
#else
  delay(2000);
#endif

  Serial.println(__FILE__);

  if (!mcp.begin()) {
    Serial.println("FAIL: begin() returned false");
    printStatus("begin");
    while (1);
  }
  Serial.println("PASS: begin() succeeded");
  printStatus("after begin");

  mcp.unlock();
  Serial.println("PASS: unlock() succeeded");
  mcp.lock(0x00);
  Serial.println("PASS: lock(0x00) succeeded");
  mcp.unlock();

  mcp.setReference(2.4f);
  Serial.println("PASS: setReference(2.4f) succeeded");

  mcp.config2(false, MCP3564::GAIN_1, MCP3564::BOOST_2);
  Serial.println("PASS: config2(GAIN_1, BOOST_2) succeeded");

  Serial.print("VREF=");
  Serial.print(mcp.getReference());
  Serial.println(" V");

  int32_t raw = mcp.analogRead(MCP3x6x_CH0);
  int32_t uV  = mcp.toVoltage(raw);
  Serial.print("PASS: analogRead(CH0)=");
  Serial.print(raw);
  Serial.print(" voltage=");
  Serial.println(uV / 1000000.0, 6);
}

void loop() {
  printStatus("status");
  delay(1000);
}

#endif  // PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// Run all tests
// ---------------------------------------------------------------------------
int runUnityTests(void) {
  UNITY_BEGIN();
  register_basic_tests();
  register_config_tests();
  register_analog_tests();
  register_scan_mode_tests();
  register_fastcmd_status_tests();
  register_misc_tests();
  register_dma_tests();

#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_hil_smoke_begin);
  RUN_TEST(test_hil_smoke_register_write);
  RUN_TEST(test_hil_smoke_status_bits);
  RUN_TEST(test_chipid_all_variants);
#endif

  return UNITY_END();
}

// ---------------------------------------------------------------------------
// Entry points
// ---------------------------------------------------------------------------

#ifdef PIO_NATIVE_TESTING
int main(int argc, char** argv) {
  suiteSetUp();
  int result = runUnityTests();
  return suiteTearDown(result);
}
#else
void loop(void) {}
void setup(void) {
#if defined(USB_VID) && defined(USB_PID)
  while (!Serial);
#else
  delay(2000);
#endif
  runUnityTests();
}
#endif
