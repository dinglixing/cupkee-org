#ifndef __TEST_INC__
#define __TEST_INC__

#include "CUnit.h"
#include "CUnit_Basic.h"

#include "mock.h"
#include "cupkee.h"

int test_cupkee_init(void);
int test_cupkee_deinit(void);

int test_cupkee_run_with_reply(const char *input, const char *expected, int try);


CU_pSuite test_hello_entry(void);
CU_pSuite test_misc_entry(void);

#endif /* __TEST_INC__ */
