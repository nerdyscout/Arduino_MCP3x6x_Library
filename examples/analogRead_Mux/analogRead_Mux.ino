/*
  analogRead_Mux

  Reads an analog input off channel0, converts it to voltage, and prints the
  result to the Serial Monitor. Attach the center pin of a potentiometer to pin
  A0, and the outside pins to +3V3 and ground.

*/

#include "MCP3x6x.hpp"

#if defined ARDUINO_AVR_PROMICRO8
MCP3561 mcp(10);
#elif defined ARDUINO_GRAND_CENTRAL_M4
SPIClass mySPI = SPIClass(&sercom5, 125, 126, 99, SPI_PAD_0_SCK_3, SERCOM_RX_PAD_2);
MCP3561 mcp(98, &mySPI);
#elif defined ADAFRUIT_METRO_M0_EXPRESS
SPIClass mySPI(&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);
MCP3561 mcp(10, &mySPI, 11, 12, 13);
// #elif
// todo: might need further cases, didn't check for all boards
#else
MCP3561 mcp;
#endif

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println(__FILE__);

  if (!mcp.begin()) {
    Serial.println("failed to initialize MCP");
    while (1)
      ;
  }
}

// the loop routine runs over and over again forever:
void loop(void) {
  int32_t adcdata;

  // read the input on default analog channel:
  do {
    adcdata = mcp.analogRead(MCP_CH0);
  } while (!mcp.status_dr());

  // Convert the analog reading
  double voltage = adcdata * mcp.getReference() / mcp.getMaxValue();

  // print out the value you read:
  Serial.print("voltage: ");
  Serial.println(voltage, 10);
}
