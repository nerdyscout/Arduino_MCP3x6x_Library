// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// MCP3462 (16-bit, 4ch) — scan mode with non-blocking polling.
// Uses dataAvailable() / readADCDATA() instead of analogRead().

#include "Arduino.h"
#include "Arduino_MCP3x6x_Library.hpp"

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
  if (mcp.dataAvailable()) {
    int32_t val    = mcp.readADCDATA();
    double voltage = val * mcp.getReference() / mcp.getMaxValue();
    Serial.print("voltage: ");
    Serial.println(voltage, 10);
  }
}
