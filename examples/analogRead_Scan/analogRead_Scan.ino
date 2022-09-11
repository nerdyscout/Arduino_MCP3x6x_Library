/*
  ReadAnalogVoltage

  Reads an analog input off channel0, converts it to voltage, and prints the
  result to the Serial Monitor. Attach the center pin of a potentiometer to pin
  A0, and the outside pins to +3V3 and ground.

*/

#include "MCP3x6x.h"

#if defined ARDUINO_AVR_PROMICRO8
MCP3561 mcp(10);
#elif defined ARDUINO_GRAND_CENTRAL_M4
SPIClass mySPI = SPIClass(&sercom5, 125, 126, 99, SPI_PAD_0_SCK_3, SERCOM_RX_PAD_2);
MCP3561 mcp(0, 0, SS, &mySPI);
#elif defined ADAFRUIT_METRO_M0_EXPRESS
SPIClass mySPI(&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);
MCP3561 mcp(8, 7, 10, &mySPI, 11, 12, 13);
// #elif
// todo: might need further cases, didn't check for all boards
#else
MCP3561 mcp;
#endif

void mcp_handler() { mcp.ISR_handler(); }

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println(__FILE__);

  if (!mcp.begin(0x03)) {
    // failed to initialize
    while (1)
      ;
  }

  attachInterrupt(digitalPinToInterrupt(8), mcp_handler, FALLING);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on default analog channel:
  int32_t adcdata0 = mcp.analogRead(0);
  int32_t adcdata1 = mcp.analogRead(1);
  // Convert the analog reading (which goes from 0 - 2^24) to a voltage (0 - 3V3):
  double voltage0 = adcdata0 * mcp.getReference() / mcp.getMaxValue();
  double voltage1 = adcdata1 * mcp.getReference() / mcp.getMaxValue();

  // print out the value you read:
  Serial.print("voltage0: ");
  Serial.println(voltage0, 10);
  Serial.print("voltage1: ");
  Serial.println(voltage1, 10);
  // pause program for one second
  delay(1000);
}
