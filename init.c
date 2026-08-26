#include "codexion.h"

int init_env(t_env *env)
{
    int i;

    env->config.simulation_running = 1;
    pthread_mutex_init(&env->config.write_mutex, NULL);
    pthread_mutex_init(&env->config.stop_mutex, NULL);
    pthread_mutex_init(&env->table_mutex, NULL);
    pthread_cond_init(&env->table_cond, NULL);

    env->coders = malloc(sizeof(t_coder) * env->config.nb_coders);
    env->dongles = malloc(sizeof(t_dongle) * env->config.nb_coders);
    if (!env->coders || !env->dongles)
        return (0);

    i = 0;
    while (i < env->config.nb_coders)
    {
        env->dongles[i].id = i + 1;
        env->dongles[i].in_use = 0;
        env->dongles[i].available_at = 0;
        if (!heap_init(&env->dongles[i].wait_queue, env->config.nb_coders))
            return (0);
        i++;
    }

    i = 0;
    while (i < env->config.nb_coders)
    {
        env->coders[i].id = i + 1;
        env->coders[i].compile_count = 0;
        env->coders[i].env = env;
        env->coders[i].left_dongle = &env->dongles[i];
        env->coders[i].right_dongle = &env->dongles[(i + 1) % env->config.nb_coders];
        i++;
    }
    return (1);
}

void cleanup_env(t_env *env)
{
    int i;

    if (env->dongles)
    {
        i = 0;
        while (i < env->config.nb_coders)
        {
            heap_free(&env->dongles[i].wait_queue);
            i++;
        }
        free(env->dongles);
    }
    if (env->coders)
        free(env->coders);
    pthread_mutex_destroy(&env->config.write_mutex);
    pthread_mutex_destroy(&env->config.stop_mutex);
    pthread_mutex_destroy(&env->table_mutex);
    pthread_cond_destroy(&env->table_cond);
}