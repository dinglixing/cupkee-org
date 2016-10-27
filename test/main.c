
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
    {"device",          native_device},
    {"config",          native_config},

    /* user native */
};

int test_cupkee_reset(void)
{
    hw_reset();

    cupkee_init();
    cupkee_set_native(native_entry, sizeof(native_entry)/sizeof(native_t));

    return 0;
}

int test_cupkee_start(const char *init)
{
    cupkee_start(init);

    return test_cupkee_run_with_reply("\r", NULL, 1);
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
    if (!test_gpio_entry()) {
        printf("Init test suite \"%s\" fail\n", "gpio");
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}

