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
  mcp.enableScanChannel(MCP3x6x_CH0);
  mcp.enableScanChannel(MCP3x6x_CH1);
  mcp.startContinuous();

  Serial.println("MCP setup done");
}

unsigned long previousMillis = 0;
const long interval          = 1000;

// the loop routine runs over and over again forever:
void loop() {
  unsigned long currentMillis = millis();
  Serial.println(currentMillis);
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // read the input on default analog channel:
    int32_t adcdata0 = mcp.analogReadContinuous(MCP3x6x_CH0);
    int32_t adcdata1 = mcp.analogReadContinuous(MCP3x6x_CH1);

    // Convert the analog reading
    double voltage0 = adcdata0 * mcp.getReference() / mcp.getMaxValue();
    double voltage1 = adcdata1 * mcp.getReference() / mcp.getMaxValue();

    // print out the value you read:
    Serial.print("voltage0: ");
    Serial.println(voltage0, 10);
    Serial.print("voltage1: ");
    Serial.println(voltage1, 10);
  }
}
