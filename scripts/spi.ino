// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout
//
// spi.ino — SPI bit-bang test sketch for Arduino Nano (ATmega328P).
// Continuously toggles SPI pins 10-13 so an oscilloscope can verify signal levels.
//
// Scope channel mapping:
//   CHAN1 = pin 10 (CS)
//   CHAN2 = pin 11 (MOSI)
//   CHAN3 = pin 12 (MISO)
//   CHAN4 = pin 13 (SCK)
//
// Connect scope probes to Nano pins 10-13 and verify toggling logic levels.
// Use 10x probes with 2V/div scale.

void setup() {
  pinMode(10, OUTPUT); // CS
  pinMode(11, OUTPUT); // MOSI
  pinMode(12, OUTPUT); // MISO
  pinMode(13, OUTPUT); // SCK
}

void loop() {
  // CS low to start transaction
  digitalWrite(10, LOW);
  delayMicroseconds(50);

  // Toggle SCK with MOSI/MISO data
  for (int i = 0; i < 8; i++) {
    digitalWrite(13, LOW);
    digitalWrite(11, i & 1);
    digitalWrite(12, !(i & 1));
    delayMicroseconds(50);
    digitalWrite(13, HIGH);
    delayMicroseconds(50);
  }

  // CS high to end transaction
  digitalWrite(10, HIGH);
  delay(100);
}
