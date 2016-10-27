#include "util.h"

void cupkee_do_callback(env_t *env, val_t *cb, uint8_t ac, val_t *av)
{
    if (val_is_native(cb)) {
        function_native_t fn = (function_native_t) val_2_intptr(cb);
        fn(env, ac, av);
    } else {
        if (ac) {
            int i;
            for (i = ac - 1; i >= 0; i--)
                env_push_call_argument(env, av + i);
        }
        env_push_call_function(env, cb);
        interp_execute_call(env, ac);
    }
}

void cupkee_error(val_t *err, env_t *env, int code)
{
    (void) env;
    (void) code;

    val_set_number(err, 1);
}

