
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

    {"pin",             native_pin},

    {"device",          native_device},
    {"enable",          native_enable},
    {"config",          native_config},
    {"write",           native_write},
    {"read",            native_read},
    {"listen",          native_listen},
    {"ignore",          native_ignore},

    /* user native */
};

int test_cupkee_reset(void)
{
    hw_dbg_reset();

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
    test_hello_entry();
    test_misc_entry();
    test_gpio_entry();

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}

