// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Stefan Herold

#include "test.h"

#ifdef PIO_NATIVE_TESTING

// ---------------------------------------------------------------------------
// DmaRingBuffer: push/pop preserves FIFO order
// ---------------------------------------------------------------------------
void test_ring_push_pop_fifo() {
  DmaSample buf[3];
  DmaRingBuffer ring;
  ring.setup(buf, 3);
  TEST_ASSERT_EQUAL(0, ring.available());

  DmaSample a = {2, 100, 111};
  DmaSample b = {3, -200, 222};
  TEST_ASSERT_TRUE(ring.push(a));
  TEST_ASSERT_TRUE(ring.push(b));
  TEST_ASSERT_EQUAL(2, ring.available());

  DmaSample out;
  TEST_ASSERT_TRUE(ring.pop(out));
  TEST_ASSERT_EQUAL(2, out.channel);
  TEST_ASSERT_EQUAL(100, out.value);
  TEST_ASSERT_EQUAL(111, out.timestamp);
  TEST_ASSERT_TRUE(ring.pop(out));
  TEST_ASSERT_EQUAL(3, out.channel);
  TEST_ASSERT_EQUAL(-200, out.value);
  TEST_ASSERT_EQUAL(222, out.timestamp);
  TEST_ASSERT_FALSE(ring.pop(out));
  TEST_ASSERT_EQUAL(0, ring.available());
}

// ---------------------------------------------------------------------------
// DmaRingBuffer: overflow drops samples without blocking
// ---------------------------------------------------------------------------
void test_ring_overflow_drops() {
  DmaSample buf[2];
  DmaRingBuffer ring;
  ring.setup(buf, 2);
  DmaSample first = {1, 10, 0};
  DmaSample second = {2, 20, 0};
  DmaSample third = {3, 30, 0};
  TEST_ASSERT_TRUE(ring.push(first));
  TEST_ASSERT_TRUE(ring.push(second));
  TEST_ASSERT_FALSE(ring.push(third));  // dropped, not blocked
  TEST_ASSERT_EQUAL(2, ring.available());

  DmaSample out;
  ring.pop(out);
  TEST_ASSERT_EQUAL(10, out.value);  // oldest survived
  ring.pop(out);
  TEST_ASSERT_EQUAL(20, out.value);
}

// ---------------------------------------------------------------------------
// DmaRingBuffer: clear empties the ring
// ---------------------------------------------------------------------------
void test_ring_clear() {
  DmaSample buf[2];
  DmaRingBuffer ring;
  ring.setup(buf, 2);
  DmaSample s = {1, 10, 0};
  ring.push(s);
  ring.clear();
  TEST_ASSERT_EQUAL(0, ring.available());
  DmaSample out;
  TEST_ASSERT_FALSE(ring.pop(out));
}

// ---------------------------------------------------------------------------
// enableDMA() rejects zero capacity
// ---------------------------------------------------------------------------
void test_enableDMA_rejects_zero_capacity() {
  TestADC adc;
  adc.begin();
  setUp();

  DmaSample buf[4];
  TEST_ASSERT_FALSE(adc.enableDMA(buf, 0));
  TEST_ASSERT_FALSE(adc.isDMAEnabled());
}

// ---------------------------------------------------------------------------
// enableDMA() accepts valid buffer and reports enabled
// ---------------------------------------------------------------------------
void test_enableDMA_success() {
  TestADC adc;
  adc.begin();
  setUp();

  DmaSample buf[4];
  TEST_ASSERT_TRUE(adc.enableDMA(buf, 4));
  TEST_ASSERT_TRUE(adc.isDMAEnabled());
  TEST_ASSERT_EQUAL(0, adc.samplesAvailable());
  DmaSample out;
  TEST_ASSERT_FALSE(adc.readSample(out));  // empty ring

  adc.disableDMA();
}

// ---------------------------------------------------------------------------
// enableDMA() rejects double enable
// ---------------------------------------------------------------------------
void test_enableDMA_twice_fails() {
  TestADC adc;
  adc.begin();
  setUp();

  DmaSample buf[4];
  DmaSample buf2[4];
  TEST_ASSERT_TRUE(adc.enableDMA(buf, 4));
  TEST_ASSERT_FALSE(adc.enableDMA(buf2, 4));
  TEST_ASSERT_TRUE(adc.isDMAEnabled());
  adc.disableDMA();
}

// ---------------------------------------------------------------------------
// disableDMA() releases state; queries return zero/false afterwards
// ---------------------------------------------------------------------------
void test_disableDMA_releases_state() {
  TestADC adc;
  adc.begin();
  setUp();

  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  adc.disableDMA();
  TEST_ASSERT_FALSE(adc.isDMAEnabled());
  TEST_ASSERT_EQUAL(0, adc.samplesAvailable());
  DmaSample out;
  TEST_ASSERT_FALSE(adc.readSample(out));
}

// ---------------------------------------------------------------------------
// samplesAvailable()/readSample() report zero/false when DMA is disabled
// ---------------------------------------------------------------------------
void test_dma_queries_disabled() {
  TestADC adc;
  adc.begin();
  setUp();

  TEST_ASSERT_EQUAL(0, adc.samplesAvailable());
  DmaSample out;
  TEST_ASSERT_FALSE(adc.readSample(out));
}

// ---------------------------------------------------------------------------
// samplesAvailable() reflects pushed samples
// ---------------------------------------------------------------------------
void test_samplesAvailable_counts() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();

  DmaSample buf[4];
  TEST_ASSERT_TRUE(adc.enableDMA(buf, 4));

  // push two samples via _dmaComplete (channel 2 and channel 3, value 0)
  memcpy(adc.dmaDataPtr(), "\x20\x00\x00\x00", 4);
  MCP3x6xTest::dmaComplete(adc, 10);
  memcpy(adc.dmaDataPtr(), "\x30\x00\x00\x00", 4);
  MCP3x6xTest::dmaComplete(adc, 20);
  TEST_ASSERT_EQUAL(2, adc.samplesAvailable());
  adc.disableDMA();
}

// ---------------------------------------------------------------------------
// _dmaComplete() decodes wire bytes and pushes an ID_SGNEXT_DATA sample
// ---------------------------------------------------------------------------
void test_dmaComplete_decodes_sample() {
  TestADC adc;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();

  DmaSample buf[4];
  adc.enableDMA(buf, 4);

  // wire order {0x20, 0, 0, 0x64} → channel 2, value 100 (same decoding as
  // readADCDATA())
  memcpy(adc.dmaDataPtr(), "\x20\x00\x00\x64", 4);
  MCP3x6xTest::dmaComplete(adc, 777);

  DmaSample out;
  TEST_ASSERT_TRUE(adc.readSample(out));
  TEST_ASSERT_EQUAL(2, out.channel);
  TEST_ASSERT_EQUAL(100, out.value);
  TEST_ASSERT_EQUAL(777, out.timestamp);
  TEST_ASSERT_EQUAL(0, adc.samplesAvailable());
  adc.disableDMA();
}

// ---------------------------------------------------------------------------
// IRQ_handler(): DMA path starts non-blocking (no SPI traffic)
// ---------------------------------------------------------------------------
void test_irq_handler_dma_nonblocking() {
  TestADC adc;
  mockStatusByte = 0x04;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();

  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  setUp();  // clear SPI tracking

  adc.IRQ_handler();
  TEST_ASSERT_TRUE(adc.dataAvailable());
  TEST_ASSERT_EQUAL(0, spiAddrs.size());  // no synchronous transfer
  adc.disableDMA();
}

// ---------------------------------------------------------------------------
// IRQ_handler(): without DMA the scan path still reads synchronously
// ---------------------------------------------------------------------------
void test_irq_handler_no_dma_sync() {
  TestADC adc;
  mockStatusByte = 0x04;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();
  setUp();

  adc.IRQ_handler();
  TEST_ASSERT_TRUE(adc.dataAvailable());
  TEST_ASSERT_TRUE(spiAddrs.size() > 0);  // synchronous transfer happened
}

// ---------------------------------------------------------------------------
// analogRead(): FIFO drain consumes other channels until requested is found
// ---------------------------------------------------------------------------
void test_analogRead_dma_fifo_drain() {
  TestADC adc;
  mockStatusByte = 0x04;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();

  DmaSample buf[4];
  adc.enableDMA(buf, 4);

  // ring order: CH1 (value 0), then CH2 (value 100)
  memcpy(adc.dmaDataPtr(), "\x10\x00\x00\x00", 4);
  MCP3x6xTest::dmaComplete(adc, 1);
  memcpy(adc.dmaDataPtr(), "\x20\x00\x00\x64", 4);
  MCP3x6xTest::dmaComplete(adc, 2);
  setUp();

  int32_t val = adc.analogRead(MCP3x6x_CH2);
  TEST_ASSERT_EQUAL(100, val);
  TEST_ASSERT_EQUAL(0, adc.samplesAvailable());  // ring drained in FIFO order
  adc.disableDMA();
}

// ---------------------------------------------------------------------------
// analogRead(): requested channel at head returns immediately
// ---------------------------------------------------------------------------
void test_analogRead_dma_head_match() {
  TestADC adc;
  mockStatusByte = 0x04;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();

  DmaSample buf[4];
  adc.enableDMA(buf, 4);

  memcpy(adc.dmaDataPtr(), "\x20\x00\x00\x42", 4);
  MCP3x6xTest::dmaComplete(adc, 5);

  int32_t val = adc.analogRead(MCP3x6x_CH2);
  TEST_ASSERT_EQUAL(0x42, val);
  TEST_ASSERT_EQUAL(0, adc.samplesAvailable());
  adc.disableDMA();
}

// ---------------------------------------------------------------------------
// analogRead(): mux mode stays synchronous even with DMA enabled
// ---------------------------------------------------------------------------
void test_analogRead_mux_still_synchronous() {
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1
  adc.begin();
  setUp();

  DmaSample buf[4];
  TEST_ASSERT_TRUE(adc.enableDMA(buf, 4));
  setUp();

  int32_t val = adc.analogRead(MCP3x6x_CH0);
  TEST_ASSERT_TRUE(spiAddrs.size() >= 2);  // MUX write + conversion (+ ADCDATA read)
  adc.disableDMA();
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_dma_tests() {
  RUN_TEST(test_ring_push_pop_fifo);
  RUN_TEST(test_ring_overflow_drops);
  RUN_TEST(test_ring_clear);
  RUN_TEST(test_enableDMA_rejects_zero_capacity);
  RUN_TEST(test_enableDMA_success);
  RUN_TEST(test_enableDMA_twice_fails);
  RUN_TEST(test_disableDMA_releases_state);
  RUN_TEST(test_dma_queries_disabled);
  RUN_TEST(test_samplesAvailable_counts);
  RUN_TEST(test_dmaComplete_decodes_sample);
  RUN_TEST(test_irq_handler_dma_nonblocking);
  RUN_TEST(test_irq_handler_no_dma_sync);
  RUN_TEST(test_analogRead_dma_fifo_drain);
  RUN_TEST(test_analogRead_dma_head_match);
  RUN_TEST(test_analogRead_mux_still_synchronous);
}

#endif  // PIO_NATIVE_TESTING
