// SPDX-License-Identifier: MIT

/**
 * @file MCPChannel.h
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.2
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef MCP_CHANNEL_H
#define MCP_CHANNEL_H

#include <Arduino.h>

#define MCP3461_DEVICE_TYPE (0x0008)  //!< MCP3461 device ID
#define MCP3462_DEVICE_TYPE (0x0009)  //!< MCP3462 device ID
#define MCP3464_DEVICE_TYPE (0x000B)  //!< MCP3464 device ID
#define MCP3561_DEVICE_TYPE (0x000C)  //!< MCP3561 device ID
#define MCP3562_DEVICE_TYPE (0x000D)  //!< MCP3562 device ID
#define MCP3564_DEVICE_TYPE (0x000F)  //!< MCP3564 device ID

#define MCP_OFFSET (0x88)  //!< corresponding mux setting
#define MCP_VCM    (0xF8)  //!< corresponding mux setting
#define MCP_AVDD   (0x98)  //!< corresponding mux setting
#define MCP_TEMP   (0xDE)  //!< corresponding mux setting
#define MCP_DIFFD  (0x67)  //!< corresponding mux setting
#define MCP_DIFFC  (0x45)  //!< corresponding mux setting
#define MCP_DIFFB  (0x23)  //!< corresponding mux setting
#define MCP_DIFFA  (0x01)  //!< corresponding mux setting
#define MCP_CH7    (0x78)  //!< corresponding mux setting
#define MCP_CH6    (0x68)  //!< corresponding mux setting
#define MCP_CH5    (0x58)  //!< corresponding mux setting
#define MCP_CH4    (0x48)  //!< corresponding mux setting
#define MCP_CH3    (0x38)  //!< corresponding mux setting
#define MCP_CH2    (0x28)  //!< corresponding mux setting
#define MCP_CH1    (0x18)  //!< corresponding mux setting
#define MCP_CH0    (0x08)  //!< corresponding mux setting

/**
 * @brief base class MCPChannel
 *
 */
class MCPChannel {
  friend class MCP3x6x;
  friend class MCPSettings;

  uint16_t _channel_mask;
  size_t _RESOLUTION_MAX;

  const uint8_t _channelID[16] = {MCP_CH0,  MCP_CH1,  MCP_CH2,   MCP_CH3,   MCP_CH4,   MCP_CH5,
                                  MCP_CH6,  MCP_CH7,  MCP_DIFFA, MCP_DIFFB, MCP_DIFFC, MCP_DIFFD,
                                  MCP_TEMP, MCP_AVDD, MCP_VCM,   MCP_OFFSET};

  /**
   * @brief latest ADC conversion result and channel
   *
   */
  struct Adcdata {
    uint8_t channelid : 4;   //!< channel ID
    int32_t value     : 25;  //!< latest value of conversion
  } _adcdata;

 protected:
  size_t _resolution;
  uint16_t id;

 public:
  /**
   * @brief structure with latest value per channel
   *
   */
  union {
    struct {
      int32_t ch[8];
      int32_t diff[4];
      int32_t temp;
      int32_t avdd;
      int32_t vcm;
      int32_t offset;
    };
    int32_t raw[16];
  } result;

  MCPChannel(const uint16_t MCP3x6x_DEVICE_TYPE);
  //  ~MCPChannel();

  size_t resolution();
  void resolution(size_t r);
};

#endif
