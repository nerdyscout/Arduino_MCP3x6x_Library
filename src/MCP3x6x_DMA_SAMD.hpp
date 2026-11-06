// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#ifndef MCP3X6X_DMA_SAMD_HPP_
#define MCP3X6X_DMA_SAMD_HPP_

#ifdef ARDUINO_ARCH_SAMD

template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
bool MCP3x6x<_MAX_RESOLUTION, _MAX_CHANNELS>::_dmaInitSAMD(uint8_t dmaChannel) {
#if defined(__SAMD21__)
  if (dmaChannel > 7) {
    return false;
  }
#elif defined(__SAMD51__)
  if (dmaChannel > 11) {
    return false;
  }
#else
  return false;
#endif
  _dmaSamdChan = dmaChannel;
  _dmaSamdActive = true;
  return true;
}

template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
void MCP3x6x<_MAX_RESOLUTION, _MAX_CHANNELS>::_dmaStartSAMD() {
  if (!_dmaSamdActive) {
    return;
  }
  // Determine the SERCOM instance used by the SPI pins
  Sercom* spiSercom = g_APinDescription[_pinCLK].ulSercom;
  if (!spiSercom) {
    return;
  }
  // Set up DMAC channel: source = SERCOM SPI DATA register,
  // destination = _dmaData. Trigger on SERCOM RX complete; transfer
  // 4 bytes (ADCDATA).
  DMAC->CHID.reg = _dmaSamdChan;
  DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;  // disable channel during config
  while (DMAC->CHCTRLA.bit.ENABLE) {
    ;  // wait for disable to complete
  }

  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
  while (DMAC->CHCTRLA.bit.SWRST) {
    ;  // wait for reset
  }

  DMAC->CHCTRLB.reg =
      DMAC_CHCTRLB_LVL(0) |              // channel priority level 0
      DMAC_CHCTRLB_EVOE |                // event output enable
      DMAC_CHCTRLB_EVACT_TRIG;           // event action: trigger

  // Source: SERCOM SPI data register (fixed address)
  DMAC->CHSRCADDR.reg = (uint32_t)&(spiSercom->SPI.DATA.reg);
  // Destination: our staging buffer (incrementing address)
  DMAC->CHDSTADDR.reg = (uint32_t)_dmaData;
  // Buffer length: 4 bytes (ADCDATA)
  DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_BEATSIZE_BYTE;
  DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_SRCINC | DMAC_CHCTRLA_DSTINC;

  // Set up the trigger source: SERCOM DMAC ID RX
  uint8_t trig = 0;
#if defined(__SAMD21__)
  // Map SERCOM instance to DMAC trigger ID
  if (spiSercom == SERCOM0) trig = SERCOM0_DMAC_ID_RX;
  else if (spiSercom == SERCOM1) trig = SERCOM1_DMAC_ID_RX;
  else if (spiSercom == SERCOM2) trig = SERCOM2_DMAC_ID_RX;
  else if (spiSercom == SERCOM3) trig = SERCOM3_DMAC_ID_RX;
  else if (spiSercom == SERCOM4) trig = SERCOM4_DMAC_ID_RX;
  else if (spiSercom == SERCOM5) trig = SERCOM5_DMAC_ID_RX;
#elif defined(__SAMD51__)
  if (spiSercom == SERCOM0) trig = SERCOM0_DMAC_ID_RX;
  else if (spiSercom == SERCOM1) trig = SERCOM1_DMAC_ID_RX;
  else if (spiSercom == SERCOM2) trig = SERCOM2_DMAC_ID_RX;
  else if (spiSercom == SERCOM3) trig = SERCOM3_DMAC_ID_RX;
  else if (spiSercom == SERCOM4) trig = SERCOM4_DMAC_ID_RX;
  else if (spiSercom == SERCOM5) trig = SERCOM5_DMAC_ID_RX;
  else if (spiSercom == SERCOM6) trig = SERCOM6_DMAC_ID_RX;
  else if (spiSercom == SERCOM7) trig = SERCOM7_DMAC_ID_RX;
#endif
  DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_TRIGSRC(trig);
  DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
  while (!DMAC->CHCTRLA.bit.ENABLE) {
    ;  // wait for enable
  }

  // Set the transfer count: 4 bytes
  DMAC->CHCTRLA.reg = (DMAC->CHCTRLA.reg & ~DMAC_CHCTRLA_BEATSIZE_Msk) |
                      DMAC_CHCTRLA_BEATSIZE_BYTE;
  DMAC->CHINTENSET.reg = DMAC_CHINTENSET_TC;  // enable transfer complete interrupt
}

template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
void MCP3x6x<_MAX_RESOLUTION, _MAX_CHANNELS>::_dmaPollSAMD() {
  if (!_dmaSamdActive) {
    return;
  }
  // Check if the DMAC channel has completed the transfer
  if (DMAC->CHINTFLAG.reg & DMAC_CHINTFLAG_TC) {
    DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TC;  // clear flag
    _dmaComplete(micros());
    // Re-arm for next conversion
    _dmaStartSAMD();
  }
}

template <const size_t _MAX_RESOLUTION, const size_t _MAX_CHANNELS>
void MCP3x6x<_MAX_RESOLUTION, _MAX_CHANNELS>::_dmaStopSAMD() {
  if (!_dmaSamdActive) {
    return;
  }
  DMAC->CHID.reg = _dmaSamdChan;
  DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
  while (DMAC->CHCTRLA.bit.ENABLE) {
    ;  // wait for disable
  }
  DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TC | DMAC_CHINTFLAG_SUSP | DMAC_CHINTFLAG_ERR;
  _dmaSamdActive = false;
}

#endif  // ARDUINO_ARCH_SAMD
#endif  // MCP3X6X_DMA_SAMD_HPP_
