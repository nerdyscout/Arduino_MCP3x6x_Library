// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// temperature sensor reading.
// Reads the internal temperature sensor channel and prints temperature in Celsius.
//
// HARDWARE: Arduino Nano (ATmega328P) + MCP3564 on default SPI pins
// WIRING:   CS=10, MOSI=11, MISO=12, SCK=13
//           No IRQ connected — polling mode
//           No MCLK connected — MCP internal oscillator

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
  double temperature = mcp.getTemperature();
  Serial.print("temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");
  delay(1000);
}
