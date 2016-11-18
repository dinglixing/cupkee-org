#include "util.h"

void cupkee_do_callback(env_t *env, val_t *cb, uint8_t ac, val_t *av)
{
    if (!cb) return;

    if (val_is_native(cb)) {
        function_native_t fn = (function_native_t) val_2_intptr(cb);
        fn(env, ac, av);
    } else
    if (val_is_script(cb)){
        if (ac) {
            int i;
            for (i = ac - 1; i >= 0; i--)
                env_push_call_argument(env, av + i);
        }
        env_push_call_function(env, cb);
        interp_execute_call(env, ac);
    }
}

val_t cupkee_error(env_t *env, int code)
{
    (void) env;

    return val_mk_number(code);
}

void cupkee_do_callback_error(env_t *env, val_t *cb, int code)
{
    val_t err = cupkee_error(env, code);

    cupkee_do_callback(env, cb, 1, &err);
}

int cupkee_id(val_t *in, int max, const char **names)
{
    if (val_is_number(in)) {
        return val_2_integer(in);
    } else {
        const char *str = val_2_cstring(in);
        if (str) {
            int id;
            for (id = 0; id < max && names[id]; id++) {
                if (!strcmp(str, names[id])) {
                    return id;
                }
            }
        }
    }
    return max;
}

