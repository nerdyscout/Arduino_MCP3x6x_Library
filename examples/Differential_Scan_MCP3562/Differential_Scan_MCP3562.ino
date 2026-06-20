// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold
//
// MCP3562 (24-bit, 4ch) — differential scan mode
// Scans differential pair DIFFA via IRQ handler, prints voltage reading.

#include "Arduino.h"
#include "MCP3x6x.h"

MCP3562 mcp;

void mcp_wrapper() { mcp.IRQ_handler(); }

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  mcp.begin();
  mcp.attachIRQ(6, mcp_wrapper);
  mcp.attachMCLK(9);

  mcp.enableScanChannel(MCP3x6x_DIFFA);
  mcp.startScan();
}

void loop() {
  int32_t adcdata = mcp.analogRead(MCP3x6x_DIFFA);
  double voltage = adcdata * mcp.getReference() / mcp.getMaxValue();
  Serial.print("voltage: ");
  Serial.println(voltage, 10);
  delay(1000);
}
