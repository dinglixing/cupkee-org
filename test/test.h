#ifndef __TEST_INC__
#define __TEST_INC__

#include "CUnit.h"
#include "CUnit_Basic.h"

#include "hardware.h"
#include "cupkee.h"

int test_cupkee_reset(void);
int test_cupkee_start(const char *init);

void test_reply_show(int on);
int test_cupkee_run_with_reply(const char *input, const char *expected, int try);
int test_cupkee_run_without_reply(const char *input, int try_max);


CU_pSuite test_hello_entry(void);
CU_pSuite test_misc_entry(void);
CU_pSuite test_gpio_entry(void);

#endif /* __TEST_INC__ */
