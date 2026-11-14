// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// SPI communication verification (HIL+SPI).
// 1) Pin-probe test first (toggle SS,SCK,MOSI,MISO) so scope channels can be
//    verified/mapped.
// 2) Deterministic SPI sequence exercising all MCP3x6x fast commands via an
//    actual MCP3x6x library instance.

#include <Arduino.h>
#include <SPI.h>

#include "Arduino_MCP3x6x_Library.hpp"

#ifndef SPI_CS_PIN
#define SPI_CS_PIN SS
#endif

// Use a concrete instance; fast-command opcodes are identical across variants.
static MCP3564 mcp;

#ifndef SKIP_PIN_PROBE
static const uint8_t pins[] = {SS, SCK, MOSI, MISO};

static void pin_probe_init() {
  for (uint8_t i = 0; i < sizeof(pins); i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }
}

// Toggle each SPI pin so wiring/channels can be verified on scope.
static void pin_probe_exec() {
  Serial.println(F("PIN_PROBE_BEGIN"));
  for (uint8_t i = 0; i < sizeof(pins); i++) {
    digitalWrite(pins[i], HIGH);
    Serial.print(F("Pin "));
    Serial.print(pins[i]);
    Serial.println(F(" is HIGH"));
    delay(1000);
    digitalWrite(pins[i], LOW);
    Serial.print(F("Pin "));
    Serial.print(pins[i]);
    Serial.println(F(" is LOW"));
    delay(1000);
  }
  Serial.println(F("PIN_PROBE_END"));
}
#endif

// Emit one MCP3x6x fast command per iteration (cycles through all 5) via the
// actual library instance at max SPI speed. One-command-per-capture keeps the
// internal scope decoder timing reliable while proving each command on-bus.
struct FastCmd {
  const char* name;
  uint8_t mosi;
};

static const FastCmd kFastCmds[] = {
  {"reset", 120}, {"conversion", 104}, {"standby", 108}, {"shutdown", 112}, {"full_shutdown", 116},
};
static uint8_t gCmdIndex = 0;

static void emitOneFastCommand(uint8_t idx) {
  switch (idx) {
    case 0:
      mcp.reset();
      break;
    case 1:
      mcp.conversion();
      break;
    case 2:
      mcp.standby();
      break;
    case 3:
      mcp.shutdown();
      break;
    case 4:
      mcp.full_shutdown();
      break;
    default:
      break;
  }
}

void setup(void) {
#if defined(USB_VID) && defined(USB_PID)
  while (!Serial)
    ;
#else
  delay(2000);
#endif
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(200);

#ifndef SKIP_PIN_PROBE
  // First test: pin probe for scope channel setup/verification.
  pin_probe_init();
  pin_probe_exec();
#endif

  // Initialize SPI for the MCP3x6x instance (fast commands only; no begin()).
  pinMode(SPI_CS_PIN, OUTPUT);
  digitalWrite(SPI_CS_PIN, HIGH);
  SPI.begin();
}

void loop(void) {
  const FastCmd& cmd = kFastCmds[gCmdIndex];
  Serial.println(F("SCOPE_BEGIN"));
  emitOneFastCommand(gCmdIndex);
  Serial.print(F("{\"name\":\""));
  Serial.print(cmd.name);
  Serial.print(F("\",\"mosi\":["));
  Serial.print(cmd.mosi);
  Serial.println(F("],\"miso\":[]}"));
  Serial.println(F("SCOPE_END"));
  gCmdIndex = (uint8_t)((gCmdIndex + 1) % 5);
  delay(400);
}
