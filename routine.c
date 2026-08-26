#include "codexion.h"

void *coder_routine(void *arg)
{
    t_coder *coder;

    coder = (t_coder *)arg;
    while (is_simulation_running(&coder->env->config))
    {
        acquire_dongles(coder);
        if (!is_simulation_running(&coder->env->config))
            break;

        coder->last_compile_start = get_time_in_ms();
        print_status(coder, "is compiling");
        precise_sleep(coder->env->config.time_to_compile);

        release_dongles(coder);
        coder->compile_count++;

        if (coder->env->config.nb_compiles_required != -1 &&
            coder->compile_count >= coder->env->config.nb_compiles_required)
            break;

        print_status(coder, "is debugging");
        precise_sleep(coder->env->config.time_to_debug);

        print_status(coder, "is refactoring");
        precise_sleep(coder->env->config.time_to_refactor);
    }
    return (NULL);
}