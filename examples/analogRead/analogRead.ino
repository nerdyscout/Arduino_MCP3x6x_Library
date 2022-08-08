/*
  ReadAnalogVoltage

  Reads an analog input off channel0, converts it to voltage, and prints the
  result to the Serial Monitor. Attach the center pin of a potentiometer to pin
  A0, and the outside pins to +3V3 and ground.

  This example code is in the public domain.
*/

#include "MCP3x6x.h"

MCP3564 mcp=MCP3564();

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  if (!mcp.begin()) {
    // failed to initialize
    while (1)
      ;
  }
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog channel 0:
  int32_t adcdata = mcp.analogRead(0x01);
  // Convert the analog reading (which goes from 0 - 2^24) to a voltage (0 - 3V3):
  double voltage = adcdata * (3.3 / ((pow(2, 24)-1)));
  // print out the value you read:
  Serial.println(voltage, 10);
  // pause program for one second
  delay(1000);
}
