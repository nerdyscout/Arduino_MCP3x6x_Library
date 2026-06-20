// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold
//
// MCP3564 (24-bit, 8ch) — mux mode with gain and oversampling.
// config2(GAIN_16) amplifies small signals, setAveraging(OSR_512) reduces noise.
// Voltage must be divided by the gain factor to get the actual input voltage.

#include "Arduino.h"
#include "MCP3x6x.h"

MCP3564 mcp;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  if (!mcp.begin()) {
    Serial.println("failed to initialize MCP");
    while (1);
  }

  mcp.config2(false, MCP3564::GAIN_16, MCP3564::BOOST_2);
  mcp.setAveraging(MCP3564::OSR_512);
}

void loop() {
  int32_t adcdata = mcp.analogRead(MCP3x6x_CH0);
  // ADC data is pre-scaled by the gain. Divide to get actual input voltage.
  double voltage = adcdata * mcp.getReference() / mcp.getMaxValue() / 16.0;
  Serial.print("voltage: ");
  Serial.println(voltage, 10);
  delay(1000);
}
