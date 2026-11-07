// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// MCP3564 — hardware-in-the-loop (HIL) smoke test.
//
// HARDWARE ONLY: requires a wired MCP3564 break-out on real SPI. The real body
// is compiled only when the build flag MCP_HIL_SMOKE is set, so CI and library
// compile-matrix builds see an intentionally empty sketch:
//
//   pio run -e SAMD21G18A --project-option 'build_flags=-DMCP_HIL_SMOKE'
//
// Exercises in one sketch:
//   - LOCK/unlock register write access
//   - CONFIG0 VREF selection
//   - CONFIG2 gain/boost configuration
//   - STATUS bits (DR / CRCCFG / POR) via status()/getDeviceAddress()
//   - voltage conversion of a mux-mode reading

#include "Arduino.h"
#include "Arduino_MCP3x6x_Library.hpp"

MCP3564 mcp;

static void printStatus(const char* where) {
  Serial.print(where);
  Serial.print(": DR=");
  Serial.print(mcp.status_dr());
  Serial.print(" CRCCFG=");
  Serial.print(mcp.status_crccfg());
  Serial.print(" POR=");
  Serial.print(mcp.status_por());
  Serial.print(" ADDR=");
  Serial.println(mcp.getDeviceAddress());
}

#if !defined(MCP_HIL_SMOKE) || !defined(ARDUINO)

// No-op sketch for CI and compile-matrix builds — this example is hardware-only.
void setup() {}
void loop() {}

#else  // MCP_HIL_SMOKE && ARDUINO

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  if (!mcp.begin()) {
    Serial.println("failed to initialize MCP");
    printStatus("begin");
    while (1);
  }

  // --- register write access ---------------------------------------------
  // LOCK=0xA5 is the POR unlock password; lock() with any other key locks.
  mcp.unlock();
  Serial.println("register map unlocked (LOCK=0xA5)");
  mcp.lock(0x00);
  Serial.println("register map locked (LOCK=0x00)");
  mcp.unlock();  // leave unlocked for the remaining configuration

  // --- CONFIG0: VREF selection --------------------------------------------
  mcp.setReference(2.4f);  // internal 2.4 V reference
  Serial.println("VREF selection written (CONFIG0.VREF_SEL=internal)");

  // --- CONFIG2: gain / boost ----------------------------------------------
  mcp.config2(false, MCP3564::GAIN_1, MCP3564::BOOST_2);
  Serial.println("CONFIG2 written (GAIN_1, BOOST_2)");
  Serial.println("VREF=");
  Serial.print(mcp.getReference());
  Serial.println(" V");
}

void loop() {
  // --- status bits ---------------------------------------------------------
  printStatus("status");

  // --- voltage conversion --------------------------------------------------
  int32_t raw = mcp.analogRead(MCP3x6x_CH0);
  int32_t uV  = mcp.toVoltage(raw);
  Serial.print("CH0 raw=");
  Serial.print(raw);
  Serial.print(" voltage=");
  Serial.println(uV / 1000000.0, 6);
  delay(1000);
}

#endif  // MCP_HIL_SMOKE && ARDUINO
