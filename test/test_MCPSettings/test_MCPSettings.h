#ifndef TEST_MCP3562_H
#define TEST_MCP3562_H

#ifdef __cplusplus
extern "C" {
#endif

void setUp(void);
void tearDown(void);
int runUnityTests(void);

void test_settings_defaults(void);

#ifdef __cplusplus
}
#endif

#endif
