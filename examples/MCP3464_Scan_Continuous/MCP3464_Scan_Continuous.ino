// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold
//
// MCP3464 (16-bit, 8ch) — continuous scan mode.
// Scans CH0-CH1 continuously via IRQ, reads with analogReadContinuous().

#include "Arduino.h"
#include "MCP3x6x.h"

MCP3464 mcp;

void mcp_wrapper() { mcp.IRQ_handler(); }

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  mcp.begin();
  mcp.attachIRQ(6, mcp_wrapper);
  mcp.attachMCLK(9);

  mcp.enableScanChannel(MCP3x6x_CH0);
  mcp.enableScanChannel(MCP3x6x_CH1);
  mcp.startScan();
  mcp.startContinuous();
}

void loop() {
  int32_t adcdata0 = mcp.analogReadContinuous(MCP3x6x_CH0);
  int32_t adcdata1 = mcp.analogReadContinuous(MCP3x6x_CH1);

  double voltage0 = adcdata0 * mcp.getReference() / mcp.getMaxValue();
  double voltage1 = adcdata1 * mcp.getReference() / mcp.getMaxValue();

  Serial.print("voltage0: ");
  Serial.println(voltage0, 10);
  Serial.print("voltage1: ");
  Serial.println(voltage1, 10);
  delay(100);
}
