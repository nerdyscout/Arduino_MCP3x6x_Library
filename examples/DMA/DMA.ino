// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// DMA continuous read.
// Uses analogReadContinuous() with DMA for continuous streaming reads.
//
// HARDWARE: SAMD21 (e.g. Arduino Zero) + MCP3564 on default SPI pins
// WIRING:   CS=10, MOSI=11, MISO=12, SCK=13
//           No IRQ connected — polling mode
//           No MCLK connected — MCP internal oscillator
//
// This example only compiles for SAMD-based boards (DMA-capable).

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

  // Enable continuous mode for CH0
  mcp.startContinuous();
}

void loop() {
  int32_t adcdata = mcp.analogReadContinuous(MCP3x6x_CH0);
  double voltage = adcdata * mcp.getReference() / mcp.getMaxValue();
  Serial.print("continuous CH0 raw=");
  Serial.print(adcdata);
  Serial.print(" voltage=");
  Serial.println(voltage, 6);
  delay(100);
}
