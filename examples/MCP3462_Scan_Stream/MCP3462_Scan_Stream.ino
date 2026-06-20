// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold
//
// MCP3462 (16-bit, 4ch) — scan mode using Stream::available() / read() API.
// Demonstrates non-blocking polling instead of analogRead().

#include "Arduino.h"
#include "MCP3x6x.h"

MCP3462 mcp;

void mcp_wrapper() { mcp.IRQ_handler(); }

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  mcp.begin();
  mcp.attachIRQ(6, mcp_wrapper);
  mcp.attachMCLK(9);

  mcp.enableScanChannel(MCP3x6x_CH0);
  mcp.startScan();
}

void loop() {
  if (mcp.available()) {
    int32_t val = mcp.read();
    double voltage = val * mcp.getReference() / mcp.getMaxValue();
    Serial.print("voltage: ");
    Serial.println(voltage, 10);
  }
}
