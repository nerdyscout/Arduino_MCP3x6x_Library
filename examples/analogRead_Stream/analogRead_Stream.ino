/*
  ReadAnalogVoltage

  Reads an analog input off channel0, converts it to voltage, and prints the
  result to the Serial Monitor. Attach the center pin of a potentiometer to pin
  A0, and the outside pins to +3V3 and ground.

  This example code is in the public domain.
*/

#include "MCP3x6x.h"

MCP3564 mcp;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  if (!mcp.begin()) {
    // failed to initialize
    while (1)
      ;
  }
// todo: enable scan mode
}

// the loop routine runs over and over again forever:
void loop() {
  Stream adcdata;
//  mcp.analogRead(&adcdata);
  Serial.println(adcdata);
}
