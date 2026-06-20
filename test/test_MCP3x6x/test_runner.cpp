// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#include "__init__.h"

// ---------------------------------------------------------------------------
// SPI tracking definitions
// ---------------------------------------------------------------------------
std::vector<uint8_t> spiAddrs;
std::vector<std::vector<uint8_t>> spiData;
uint8_t mockStatusByte = 0x00;

bool digitalWriteCalled = false;
uint8_t digitalWritePin = 0xFF;
uint8_t digitalWriteValue = 0xFF;

bool pinModeCalled = false;
uint8_t pinModePin = 0xFF;
uint8_t pinModeMode = 0xFF;

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
        spiAddrs.push_back(addr);
        return mockStatusByte;
      });

  When(OverloadedMethod(ArduinoFake(SPI), transfer, void(void*, size_t)))
      .AlwaysDo([](void* data, size_t size) {
        std::vector<uint8_t> bytes(size);
        std::memcpy(bytes.data(), data, size);
        spiData.push_back(bytes);
      });
#endif
}

int suiteTearDown(int num_failures) { return num_failures; }

void setUp(void) {
  spiAddrs.clear();
  spiData.clear();
  digitalWriteCalled = false;
  digitalWritePin = 0xFF;
  digitalWriteValue = 0xFF;
  pinModeCalled = false;
  pinModePin = 0xFF;
  pinModeMode = 0xFF;
}

void tearDown(void) {}

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
    while (!Serial) {}
  #else
    delay(2000);
  #endif
  runUnityTests();
}
#endif
