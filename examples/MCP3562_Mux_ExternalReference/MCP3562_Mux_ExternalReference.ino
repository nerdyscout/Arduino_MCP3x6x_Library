// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold
//
// MCP3562 (24-bit, 4ch) — mux mode with external reference voltage.
// setReference() configures the ADC for an external VREF (e.g. 3.3V).

#include "Arduino.h"
#include "MCP3x6x.h"

MCP3562 mcp;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  if (!mcp.begin()) {
    Serial.println("failed to initialize MCP");
    while (1);
  }

  mcp.setReference(3.3);  // External 3.3V reference
}

void loop() {
  int32_t adcdata = mcp.analogRead(MCP3x6x_CH0);
  double voltage = adcdata * mcp.getReference() / mcp.getMaxValue();
  Serial.print("voltage: ");
  Serial.println(voltage, 10);
  delay(1000);
}
