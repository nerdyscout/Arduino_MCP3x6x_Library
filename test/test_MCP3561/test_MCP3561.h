// SPDX-License-Identifier: MIT

/**
 * @file test_MCP3561.h
 * @author Stefan Herold (stefan.herold@posteo.de)
 * @brief
 * @version 0.0.2
 * @date 2023-10-10
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef TEST_TEST_MCP3561_TEST_MCP3561_H_
#define TEST_TEST_MCP3561_TEST_MCP3561_H_

int runUnityTests(void);

void suiteSetUp(void);
void suiteTearDown(void);

void setUp(void);
void tearDown(void);

void test_instance(void);

#endif  // TEST_TEST_MCP3561_TEST_MCP3561_H_
