#include <Arduino.h>

#include "Arduino_MCP3x6x_Library.hpp"

MCP3561 mcp;

void setup() { Serial.begin(SERIAL_BAUD); }

void loop() {
  delay(3000);

  bool b = mcp.begin();
  Serial.print(" begin:");
  Serial.print(String(b, BIN));

  while (1) {
    byte status = mcp.status();
    Serial.print(" Status:");
    Serial.print(String(status, BIN));

    Serial.print(" Read:");
    Serial.print(String(mcp.read()));

    Serial.println();
    delay(1);
  }
}