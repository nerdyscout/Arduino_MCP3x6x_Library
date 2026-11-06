// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#ifndef MCP3X6X_DMA_HPP_
#define MCP3X6X_DMA_HPP_

#include <cstddef>

/**
 * @brief single sample captured by the DMA engine
 *
 */
struct DmaSample {
  uint8_t channel;    //!< channel ID decoded from ADCDATA[31:28] (ID_SGNEXT_DATA)
  int32_t value;      //!< sign-extended ADC value
  uint32_t timestamp; //!< micros()/millis() when the sample was captured
};

/**
 * @brief lock-free single-producer/single-consumer ring buffer for DMA samples
 *
 * The DMA completion interrupt produces (push), the user context consumes
 * (pop). Storage is caller-provided; no heap is used. Pushing into a full
 * buffer drops the sample (returns false) — it never blocks.
 */
class DmaRingBuffer {
 public:
  void setup(DmaSample* buf, size_t cap) {
    _buf = buf;
    _cap = cap;
    clear();
  }
  bool push(DmaSample s) {
    if (_buf == nullptr || _count >= _cap) {
      return false;
    }
    _buf[(_head + _count) % _cap] = s;
    _count++;
    return true;
  }
  bool pop(DmaSample& s) {
    if (_buf == nullptr || _count == 0) {
      return false;
    }
    s = _buf[_head];
    _head = (_head + 1) % _cap;
    _count--;
    return true;
  }
  size_t available() const { return _count; }
  void clear() {
    _head = 0;
    _count = 0;
  }

 private:
  DmaSample* _buf = nullptr;
  size_t _cap = 0;
  volatile size_t _head = 0;
  volatile size_t _count = 0;
};

#endif  // MCP3X6X_DMA_HPP_
