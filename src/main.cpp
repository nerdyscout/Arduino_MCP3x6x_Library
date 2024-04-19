
#include "main.hpp"

#include <Arduino.h>
#include <MCP3x6x.h>

SPIClass mySPI(&sercom1, 12, 13, 11, SPI_PAD_0_SCK_1, SERCOM_RX_PAD_3);
MCP3561 mcp(10, &mySPI, SPISettings(1000000, MSBFIRST, SPI_MODE0), 11, 12, 13);
// MCP3562 mcp(10, &SPI, SPISettings(1000000, MSBFIRST, SPI_MODE0), 11, 12, 13);

void mcp_wrapper() { mcp.IRQ_handler(); }

void setup() {
  Serial.begin(115200);
  mcp.begin();

  mcp.configure();

  mcp.config0(MCP3x6x::STANDBY);
  //  mcp.attachIRQ(8, mcp_wrapper);
}

void loop() {
  //  int32_t adcdata0        = mcp.analogRead(MCP3x6x_CH0);
  //  int32_t adcdata1        = mcp.analogRead(MCP3x6x_CH1);

  //  MCP3x6x::Config0 buffer = 0;
  //  mcp.read(buffer);
  //  Serial.println(buffer.raw, HEX);
  /*
    Serial.print("ADC0: ");
    Serial.println(adcdata0);
    Serial.print("ADC1: ");
    Serial.println(adcdata1);
  */
  delay(1000);
}
