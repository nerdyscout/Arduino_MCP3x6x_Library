// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// differential mux mode
// Reads CH0-CH1 differential pair, converts to voltage, prints to serial.

#include "Arduino.h"
#include "Arduino_MCP3x6x_Library.hpp"

MCP3564 mcp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println(__FILE__);

  if (!mcp.begin()) {
    Serial.println("failed to initialize MCP");
    while (1)
      ;
  }
}

void loop() {
  int32_t adcdata = mcp.analogReadDifferential(MCP3564::MUX_CH0, MCP3564::MUX_CH1);
  double voltage = adcdata * mcp.getReference() / mcp.getMaxValue();
  Serial.print("voltage: ");
  Serial.println(voltage, 10);
  delay(1000);
}
