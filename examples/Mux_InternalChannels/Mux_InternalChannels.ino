// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// read internal temperature sensor and AVDD voltage.
// No external wiring needed — purely diagnostic.

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
  int32_t temp = mcp.analogRead(MCP3x6x_TEMP);
  int32_t avdd = mcp.analogRead(MCP3x6x_AVDD);

  double voltage = avdd * mcp.getReference() / mcp.getMaxValue();

  Serial.print("temperature raw: ");
  Serial.print(temp);
  Serial.print("  AVDD: ");
  Serial.print(voltage, 6);
  Serial.println(" V");
  delay(1000);
}
