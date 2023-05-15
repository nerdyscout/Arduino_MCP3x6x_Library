// SPDX-License-Identifier: MIT

/**
 * @file MCPChannel.cpp
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.2
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "MCPChannel.hpp"

#include <Arduino.h>

MCPChannel::MCPChannel(const uint16_t MCP3x6x_DEVICE_TYPE) { id = MCP3x6x_DEVICE_TYPE; }

size_t MCPChannel::resolution() { return _resolution; }

void MCPChannel::resolution(size_t r) { _resolution = r; }
