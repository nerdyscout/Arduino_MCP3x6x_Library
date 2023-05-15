// SPDX-License-Identifier: MIT

/**
 * @file MCPSettings.cpp
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.2
 * @date 2023-04-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "MCPSettings.hpp"

#include <Arduino.h>
#include <string.h>

MCPSettings::MCPSettings(const config0_t CONFIG0, const config1_t CONFIG1, const config2_t CONFIG2,
                         const config3_t CONFIG3, const irq_t IRQ, const mux_t MUX,
                         const scan_t SCAN, const timer_t TIMER, const offset_t OFFSET,
                         const gain_t GAIN, const lock_t LOCK, const crccfg_t CRCCFG) {
  config0 = CONFIG0;
  config1 = CONFIG1;
  config2 = CONFIG2;
  irq     = IRQ;
  mux     = MUX;
  memcpy(scan.raw, SCAN.raw, 3);
  memcpy(timer.raw, TIMER.raw, 3);
  memcpy(offsetcal.raw, OFFSET.raw, 3);
  memcpy(gaincal.raw, GAIN.raw, 3);
  lock = LOCK;
  memcpy(crccfg.raw, CRCCFG.raw, 2);
}

MCPSettings::MCPSettings(const uint8_t CONFIG0, const uint8_t CONFIG1, const uint8_t CONFIG2,
                         const uint8_t CONFIG3, const uint8_t IRQ, const uint8_t MUX,
                         const uint8_t* SCAN, const uint8_t* TIMER, const uint8_t* OFFSET,
                         const uint8_t* GAIN, const uint8_t LOCK, const uint8_t* CRCCFG) {
  config0.raw = CONFIG0;
  config1.raw = CONFIG1;
  config2.raw = CONFIG2;
  irq.raw     = IRQ;
  mux.raw     = MUX;
  memcpy(scan.raw, SCAN, 3);
  memcpy(timer.raw, TIMER, 3);
  memcpy(offsetcal.raw, OFFSET, 3);
  memcpy(gaincal.raw, GAIN, 3);
  lock.raw = LOCK;
  memcpy(crccfg.raw, CRCCFG, 2);
}

void MCPSettings::reset() {
  config0.raw = _DEFAULT_CONFIG0;
  config1.raw = _DEFAULT_CONFIG1;
  config2.raw = _DEFAULT_CONFIG2;
  config3.raw = _DEFAULT_CONFIG3;
  irq.raw     = _DEFAULT_IRQ;
  mux.raw     = _DEFAULT_MUX;
  memcpy(scan.raw, _DEFAULT_SCAN, 3);
  memcpy(timer.raw, _DEFAULT_TIMER, 3);
  memcpy(offsetcal.raw, _DEFAULT_OFFSET, 3);
  memcpy(gaincal.raw, _DEFAULT_GAIN, 3);
  lock.raw = _DEFAULT_LOCK;
  memcpy(crccfg.raw, _DEFAULT_CRCCFG, 2);
}
