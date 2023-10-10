// SPDX-License-Identifier: MIT

/**
 * @file test_MCP3x6x.h
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.2
 * @date 2023-10-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef TEST_TEST_MCP3X6X_H_
#define TEST_TEST_MCP3X6X_H_

#ifdef __cplusplus
extern "C" {
#endif

void setUp(void);
void tearDown(void);
int runUnityTests(void);

// tests
void test_instance(void);

#ifdef __cplusplus
}
#endif

#endif  // TEST_TEST_MCP3X6X_H_
