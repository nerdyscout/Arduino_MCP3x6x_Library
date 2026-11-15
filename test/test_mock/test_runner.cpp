// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"
#include "tests.hpp"

// ---------------------------------------------------------------------------
// SPI tracking (mocked)
// ---------------------------------------------------------------------------
std::vector<uint8_t> spiAddrs;
std::vector<std::vector<uint8_t>> spiData;
std::vector<uint8_t> mockReadSequence;
uint8_t mockStatusByte = 0x00;
uint8_t lastSpiAddress = 0x00;

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
  ArduinoFakeReset();

  When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
  When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
  When(Method(ArduinoFake(), delay)).AlwaysReturn();
  When(Method(ArduinoFake(), millis)).AlwaysReturn(0);
  When(Method(ArduinoFake(), attachInterrupt)).AlwaysReturn();
  When(Method(ArduinoFake(), detachInterrupt)).AlwaysReturn();
  When(Method(ArduinoFake(), noTone)).AlwaysReturn();
  When(Method(ArduinoFake(), sei)).AlwaysReturn();
  When(Method(ArduinoFake(), cli)).AlwaysReturn();

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
}

int suiteTearDown(int num_failures) {
  return num_failures;
}

void setUp(void) {
  spiAddrs.clear();
  spiData.clear();
  mockReadSequence.clear();
  digitalWriteCalled = false;
  digitalWritePin = 0xFF;
  digitalWriteValue = 0xFF;
  pinModeCalled = false;
  pinModePin = 0xFF;
  pinModeMode = 0xFF;
}

void tearDown(void) {
}

int runUnityTests(void) {
  UNITY_BEGIN();
  register_init_tests();
  register_config_tests();
  register_conversion_tests();
  register_status_tests();
  register_dma_tests();
  register_calibration_tests();
  register_spi_tests();
  return UNITY_END();
}

int main(int argc, char** argv) {
  suiteSetUp();
  int result = runUnityTests();
  return suiteTearDown(result);
}
