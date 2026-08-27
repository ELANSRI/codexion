#include "codexion.h"

void *coder_routine(void *arg)
{
    t_coder *coder;
    int     done_compiles;

    coder = (t_coder *)arg;
    while (is_simulation_running(&coder->env->config))
    {
        acquire_dongles(coder);
        if (!is_simulation_running(&coder->env->config))
            break;
        if (coder->left_dongle == coder->right_dongle)
            continue;

        pthread_mutex_lock(&coder->state_mutex);
        coder->last_compile_start = get_time_in_ms();
        pthread_mutex_unlock(&coder->state_mutex);
        print_status(coder, "is compiling");
        precise_sleep(coder->env->config.time_to_compile);

        release_dongles(coder);
        pthread_mutex_lock(&coder->state_mutex);
        coder->compile_count++;
        done_compiles = coder->compile_count;
        pthread_mutex_unlock(&coder->state_mutex);

        if (coder->env->config.nb_compiles_required != -1 &&
            done_compiles >= coder->env->config.nb_compiles_required)
            break;

        print_status(coder, "is debugging");
        precise_sleep(coder->env->config.time_to_debug);

        print_status(coder, "is refactoring");
        precise_sleep(coder->env->config.time_to_refactor);
    }
    return (NULL);
}