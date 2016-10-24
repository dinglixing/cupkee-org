
#include "test.h"

static const native_t native_entry[] = {
    /* cupkee native */
    {"sysinfos",        native_sysinfos},
    {"systicks",        native_systicks},

    {"print",           native_print},
    {"led",             native_led},

    {"setTimeout",      native_set_timeout},
    {"setInterval",     native_set_interval},
    {"clearTimeout",    native_clear_timeout},
    {"clearInterval",   native_clear_interval},

    {"scripts",         native_scripts},

    /* user native */
};

int test_cupkee_init(void)
{
    cupkee_init();

    cupkee_set_native(native_entry, sizeof(native_entry)/sizeof(native_t));
    cupkee_start();

    return 0;
}

int test_cupkee_deinit(void)
{
    return 0;
}

int main(int argc, const char *argv[])
{
    (void) argc;
    (void) argv;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    // add test suite here:
    if (!test_hello_entry()) {
        printf("Init test suite \"%s\" fail\n", "hello");
    }
    if (!test_misc_entry()) {
        printf("Init test suite \"%s\" fail\n", "misc");
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}

