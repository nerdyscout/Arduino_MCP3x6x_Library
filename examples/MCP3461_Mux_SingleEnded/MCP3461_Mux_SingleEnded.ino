// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold
//
// MCP3461 (16-bit, 2ch) — single-ended mux mode
// Reads CH0, converts to voltage, prints to serial.

#include "Arduino.h"
#include "MCP3x6x.h"

MCP3461 mcp;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  if (!mcp.begin()) {
    Serial.println("failed to initialize MCP");
    while (1);
  }
}

void loop() {
  int32_t adcdata = mcp.analogRead(MCP3x6x_CH0);
  double voltage = adcdata * mcp.getReference() / mcp.getMaxValue();
  Serial.print("voltage: ");
  Serial.println(voltage, 10);
  delay(1000);
}
