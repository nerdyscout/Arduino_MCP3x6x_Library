// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#ifndef MCP3X6X_DMA_ESP32_HPP_
#define MCP3X6X_DMA_ESP32_HPP_

#ifdef ARDUINO_ARCH_ESP32

#include "driver/spi_master.h"

template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
bool MCP3x6x<_MAX_RESOLUTION, _MAX_CHANNELS>::_dmaInitESP32(uint8_t dmaChannel) {
  spi_host_device_t host = (dmaChannel == 1) ? SPI3_HOST : SPI2_HOST;
  spi_device_interface_config_t devcfg = {};
  devcfg.clock_speed_hz = 1000000;  // 1 MHz: safe default for ADCDATA reads
  devcfg.mode = 0;
  devcfg.spics_io_num = (int)_pinCS;
  devcfg.queue_size = 1;
  devcfg.flags = 0;
  esp_err_t err = spi_bus_add_device(host, &devcfg, &_dmaEsp32Handle);
  return (err == ESP_OK);
}

template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
void MCP3x6x<_MAX_RESOLUTION, _MAX_CHANNELS>::_dmaStartESP32() {
  if (!_dmaEsp32Handle) {
    return;
  }
  uint8_t addr = MCP3x6x_CMD_SREAD | MCP3x6x_ADR_ADCDATA;
  _dmaEsp32Trans = {};
  _dmaEsp32Trans.length = 8;          // 1 byte: address/command
  _dmaEsp32Trans.rxlength = 32;       // 4 bytes: ADCDATA payload
  _dmaEsp32Trans.tx_buffer = &addr;
  _dmaEsp32Trans.rx_buffer = _dmaData;
  _dmaEsp32Trans.flags = 0;
  spi_device_queue_trans(_dmaEsp32Handle, &_dmaEsp32Trans, portMAX_DELAY);
}

template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
void MCP3x6x<_MAX_RESOLUTION, _MAX_CHANNELS>::_dmaPollESP32() {
  if (!_dmaEsp32Handle) {
    return;
  }
  spi_transaction_t* trans = nullptr;
  esp_err_t err = spi_device_get_trans_result(_dmaEsp32Handle, &trans, 0);
  if (err == ESP_OK && trans) {
    _dmaComplete(micros());
    // Re-arm for the next conversion: queue another transaction
    _dmaStartESP32();
  }
}

template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
void MCP3x6x<_MAX_RESOLUTION, _MAX_CHANNELS>::_dmaStopESP32() {
  if (_dmaEsp32Handle) {
    spi_bus_remove_device(_dmaEsp32Handle);
    _dmaEsp32Handle = nullptr;
  }
  _dmaEsp32Trans = {};
}

#endif  // ARDUINO_ARCH_ESP32
#endif  // MCP3X6X_DMA_ESP32_HPP_
