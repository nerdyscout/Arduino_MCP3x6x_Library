// SPDX-License-Identifier: MIT
// Copyright (c) 2026 nerdyscout

#ifndef TESTS_HPP_
#define TESTS_HPP_

// Aggregates all register_<group>() declarations so test_runner.cpp has a
// single point of truth for test registration.

void register_init_tests();
void register_config_tests();
void register_conversion_tests();
void register_status_tests();
void register_dma_tests();
void register_calibration_tests();
void register_spi_tests();

#endif  // TESTS_HPP_
