#include "codexion.h"

int main(int argc, char **argv)
{
    t_env       env;
    int         i;
    pthread_t   monitor_thread;

    memset(&env, 0, sizeof(t_env));
    if (!parse_args(argc, argv, &env.config))
        return (1);
    if (!init_env(&env))
    {
        cleanup_env(&env);
        return (1);
    }

    env.config.start_time = get_time_in_ms();
    i = 0;
    while (i < env.config.nb_coders)
    {
        env.coders[i].last_compile_start = env.config.start_time;
        if (pthread_create(&env.coders[i].thread_id, NULL, &coder_routine, &env.coders[i]) != 0)
        {
            stop_simulation(&env.config);
            break;
        }
        i++;
    }

    pthread_create(&monitor_thread, NULL, &monitor_routine, &env);

    pthread_join(monitor_thread, NULL);
    i = 0;
    while (i < env.config.nb_coders)
    {
        pthread_join(env.coders[i].thread_id, NULL);
        i++;
    }

    cleanup_env(&env);
    return (0);
}