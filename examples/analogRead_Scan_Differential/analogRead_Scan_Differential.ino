/*
  ReadAnalogVoltage

  Reads an analog input off channel0, converts it to voltage, and prints the
  result to the Serial Monitor. Attach the center pin of a potentiometer to pin
  A0, and the outside pins to +3V3 and ground.

*/

#include "Arduino.h"
#include "MCP3x6x.h"

MCP3561 mcp;

void mcp_wrapper() { mcp.IRQ_handler(); }

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(__FILE__);

  mcp.begin();
  mcp.attachIRQ(6, mcp_wrapper);

  mcp.enableScanChannel(MCP3x6x_DIFFA);
  mcp.startScan();
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on default analog channel:
  int32_t adcdata = mcp.analogRead(MCP3x6x_DIFFA);

  // Convert the analog reading
  double voltage = adcdata * mcp.getReference() / mcp.getMaxValue();

  // print out the value you read:
  Serial.print("voltage: ");
  Serial.println(voltage, 10);
  // pause program for one second
  delay(1000);
}
