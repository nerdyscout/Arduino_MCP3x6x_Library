// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// MCP3564 — smoke test, polling mode, default SPI pins.
//
// HARDWARE: Arduino Nano (ATmega328P) + MCP3564 24-bit ADC
// WIRING:   CS=10, MOSI=11, MISO=12, SCK=13 (library defaults)
//           No IRQ connected — polling mode
//           No MCLK connected — MCP internal oscillator
//           Analog inputs floating
//
// begin() reads the chip ID back via _validateChipID(). MCP3564 expects
// nibble 0xF (24-bit, MCP3564 variant). If begin() returns false, the part
// is not MCP3564 — change the class below and re-upload.
//
// Note: MCP3564 in this library = MCP3x6x<24, 8>. The library's _MAX_CHANNELS
// parameter maps to the chip-ID nibble expected by _validateChipID().

#include "Arduino.h"
#include "Arduino_MCP3x6x_Library.hpp"

// Instantiate MCP3564 (MCP3x6x<24, 8>). Default constructor uses
// SS/MOSI/MISO/SCK = 10/11/12/13 on the Nano.
MCP3564 mcp;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  Serial.print("begin() ... ");
  bool ok = mcp.begin();
  Serial.println(ok ? "OK" : "FAILED");

  if (!ok) {
    Serial.println("Chip ID mismatch or reset failed.");
    Serial.println("This sketch uses MCP3564 (expects ID nibble 0xF).");
    // Don't hang — loop() will keep retrying begin() for scope capture
  }

  Serial.print("Device address: 0x");
  Serial.println(mcp.getDeviceAddress(), HEX);
  Serial.println("Retrying begin() every 500ms for scope capture.");
  Serial.println();
}

void loop() {
  // Continuously retry begin() to generate repeatable SPI traffic for scope capture
  Serial.print("begin() ... ");
  bool ok = mcp.begin();
  Serial.println(ok ? "OK" : "FAILED");
  delay(500);
}
