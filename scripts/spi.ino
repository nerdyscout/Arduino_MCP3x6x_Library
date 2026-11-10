// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// spi.ino — SPI bit-bang test sketch for Arduino Nano (ATmega328P).
// Continuously sends SPI bytes 0xA5 on MOSI so an oscilloscope SPI decoder
// can capture and decode frames.
//
// Scope channel mapping:
//   CHAN1 = pin 10 (CS)
//   CHAN2 = pin 11 (MOSI)
//   CHAN3 = pin 12 (MISO)
//   CHAN4 = pin 13 (SCK)
//
// Connect scope probes to Nano pins 10-13 and verify decoding.
// Use 10x probes with 2V/div, 100ns/div timebase.

void setup() {
  pinMode(10, OUTPUT); // CS
  pinMode(11, OUTPUT); // MOSI
  pinMode(12, OUTPUT); // MISO
  pinMode(13, OUTPUT); // SCK
}

void loop() {
  // Send 0xA5 (10100101) on MOSI
  digitalWrite(10, LOW);   // CS low
  delayMicroseconds(10);
  for (int bit = 7; bit >= 0; bit--) {
    digitalWrite(13, LOW);   // SCK low
    digitalWrite(11, (0xA5 >> bit) & 1); // MOSI bit
    delayMicroseconds(50);
    digitalWrite(13, HIGH);  // SCK high
    delayMicroseconds(50);
  }
  digitalWrite(10, HIGH);  // CS high
  delay(100);
}
