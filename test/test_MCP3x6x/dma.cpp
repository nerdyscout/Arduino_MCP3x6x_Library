// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#include "test.h"

// ---------------------------------------------------------------------------
// DmaRingBuffer: FIFO order
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
  DmaSample first  = {1, 10, 0};
  DmaSample second = {2, 20, 0};
  DmaSample third  = {3, 30, 0};
  TEST_ASSERT_TRUE(ring.push(first));
  TEST_ASSERT_TRUE(ring.push(second));
  TEST_ASSERT_FALSE(ring.push(third));  // dropped, not blocked
  TEST_ASSERT_EQUAL(2, ring.available());

  DmaSample out;
  ring.pop(out);
  TEST_ASSERT_EQUAL(10, out.value);
  ring.pop(out);
  TEST_ASSERT_EQUAL(20, out.value);
}

// ---------------------------------------------------------------------------
// DmaRingBuffer: clear empties
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
// enableDMA — zero capacity rejected
// ---------------------------------------------------------------------------
void test_enable_dma_zero_capacity_rejected() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  DmaSample buf[4];
  TEST_ASSERT_FALSE(adc.enableDMA(buf, 0));
  TEST_ASSERT_FALSE(adc.isDMAEnabled());
#else
  TestADC adc;
  adc.begin();
  DmaSample buf[4];
  adc.enableDMA(buf, 0);
  adc.disableDMA();
#endif
}

// ---------------------------------------------------------------------------
// enableDMA — double enable rejected
// ---------------------------------------------------------------------------
void test_enable_dma_double_enable_rejected() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  DmaSample buf[4];
  DmaSample buf2[4];
  TEST_ASSERT_TRUE(adc.enableDMA(buf, 4));
  TEST_ASSERT_FALSE(adc.enableDMA(buf2, 4));
  TEST_ASSERT_TRUE(adc.isDMAEnabled());
  adc.disableDMA();
#else
  TestADC adc;
  adc.begin();
  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  adc.disableDMA();
#endif
}

// ---------------------------------------------------------------------------
// enableDMA — success
// ---------------------------------------------------------------------------
void test_enable_dma_success() {
#ifdef PIO_NATIVE_TESTING
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
#else
  TestADC adc;
  adc.begin();
  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  adc.disableDMA();
#endif
}

// ---------------------------------------------------------------------------
// disableDMA — releases state
// ---------------------------------------------------------------------------
void test_disable_dma_releases_state() {
#ifdef PIO_NATIVE_TESTING
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
#else
  TestADC adc;
  adc.begin();
  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  adc.disableDMA();
#endif
}

// ---------------------------------------------------------------------------
// queries when DMA disabled
// ---------------------------------------------------------------------------
void test_dma_queries_disabled() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  TEST_ASSERT_EQUAL(0, adc.samplesAvailable());
  DmaSample out;
  TEST_ASSERT_FALSE(adc.readSample(out));
#else
  TestADC adc;
  adc.begin();
  adc.samplesAvailable();
  DmaSample out;
  adc.readSample(out);
#endif
}

// ---------------------------------------------------------------------------
// samplesAvailable reflects pushed samples
// ---------------------------------------------------------------------------
void test_samples_available_counts() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();

  DmaSample buf[4];
  TEST_ASSERT_TRUE(adc.enableDMA(buf, 4));

  memcpy(adc.dmaDataPtr(), "\x20\x00\x00\x00", 4);
  MCP3x6xTest::dmaComplete(adc, 10);
  memcpy(adc.dmaDataPtr(), "\x30\x00\x00\x00", 4);
  MCP3x6xTest::dmaComplete(adc, 20);
  TEST_ASSERT_EQUAL(2, adc.samplesAvailable());
  adc.disableDMA();
#else
  TestADC adc;
  adc.begin();
  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  adc.disableDMA();
#endif
}

// ---------------------------------------------------------------------------
// _dmaComplete decodes wire bytes
// ---------------------------------------------------------------------------
void test_dma_complete_decodes_sample() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();

  DmaSample buf[4];
  adc.enableDMA(buf, 4);

  memcpy(adc.dmaDataPtr(), "\x20\x00\x00\x64", 4);
  MCP3x6xTest::dmaComplete(adc, 777);

  DmaSample out;
  TEST_ASSERT_TRUE(adc.readSample(out));
  TEST_ASSERT_EQUAL(2, out.channel);
  TEST_ASSERT_EQUAL(100, out.value);
  TEST_ASSERT_EQUAL(777, out.timestamp);
  TEST_ASSERT_EQUAL(0, adc.samplesAvailable());
  adc.disableDMA();
#else
  TestADC adc;
  adc.begin();
  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  adc.disableDMA();
#endif
}

// ---------------------------------------------------------------------------
// IRQ_handler: DMA path starts non-blocking (no SPI traffic)
// ---------------------------------------------------------------------------
void test_irq_handler_dma_no_spi() {
#ifdef PIO_NATIVE_TESTING
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
#else
  TestADC adc;
  adc.begin();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();
  adc.IRQ_handler();
#endif
}

// ---------------------------------------------------------------------------
// IRQ_handler: no DMA → synchronous read
// ---------------------------------------------------------------------------
void test_irq_handler_no_dma_sync_read() {
#ifdef PIO_NATIVE_TESTING
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
#else
  TestADC adc;
  adc.begin();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();
  adc.IRQ_handler();
#endif
}

// ---------------------------------------------------------------------------
// analogRead DMA FIFO drain
// ---------------------------------------------------------------------------
void test_analog_read_dma_fifo_drain() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x04;
  adc.begin();
  setUp();
  adc.enableScanChannel(MCP3x6x_CH0);
  adc.startScan();

  DmaSample buf[4];
  adc.enableDMA(buf, 4);

  memcpy(adc.dmaDataPtr(), "\x10\x00\x00\x00", 4);
  MCP3x6xTest::dmaComplete(adc, 1);
  memcpy(adc.dmaDataPtr(), "\x20\x00\x00\x64", 4);
  MCP3x6xTest::dmaComplete(adc, 2);
  setUp();

  int32_t val = adc.analogRead(MCP3x6x_CH2);
  TEST_ASSERT_EQUAL(100, val);
  TEST_ASSERT_EQUAL(0, adc.samplesAvailable());  // ring drained in FIFO order
  adc.disableDMA();
#else
  TestADC adc;
  adc.begin();
  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  adc.disableDMA();
#endif
}

// ---------------------------------------------------------------------------
// analogRead DMA head match
// ---------------------------------------------------------------------------
void test_analog_read_dma_head_match() {
#ifdef PIO_NATIVE_TESTING
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
#else
  TestADC adc;
  adc.begin();
  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  adc.disableDMA();
#endif
}

// ---------------------------------------------------------------------------
// analogRead mux mode stays synchronous even with DMA enabled
// ---------------------------------------------------------------------------
void test_analog_read_mux_still_synchronous() {
#ifdef PIO_NATIVE_TESTING
  TestADC adc;
  mockStatusByte = 0x04;  // DR=1
  adc.begin();
  setUp();

  DmaSample buf[4];
  TEST_ASSERT_TRUE(adc.enableDMA(buf, 4));
  setUp();

  int32_t val = adc.analogRead(MCP3x6x_CH0);
  TEST_ASSERT_TRUE(spiAddrs.size() >= 2);  // MUX write + conversion
  adc.disableDMA();
#else
  TestADC adc;
  adc.begin();
  DmaSample buf[4];
  adc.enableDMA(buf, 4);
  adc.analogRead(MCP3x6x_CH0);
  adc.disableDMA();
#endif
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
void register_dma_tests() {
  RUN_TEST(test_ring_push_pop_fifo);
  RUN_TEST(test_ring_overflow_drops);
  RUN_TEST(test_ring_clear);
#ifdef PIO_NATIVE_TESTING
  RUN_TEST(test_enable_dma_zero_capacity_rejected);
  RUN_TEST(test_enable_dma_double_enable_rejected);
  RUN_TEST(test_enable_dma_success);
  RUN_TEST(test_disable_dma_releases_state);
  RUN_TEST(test_dma_queries_disabled);
  RUN_TEST(test_samples_available_counts);
  RUN_TEST(test_dma_complete_decodes_sample);
  RUN_TEST(test_irq_handler_dma_no_spi);
  RUN_TEST(test_irq_handler_no_dma_sync_read);
  RUN_TEST(test_analog_read_dma_fifo_drain);
  RUN_TEST(test_analog_read_dma_head_match);
  RUN_TEST(test_analog_read_mux_still_synchronous);
#endif
}
