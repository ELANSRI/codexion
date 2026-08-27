#include "codexion.h"

int init_env(t_env *env)
{
    int i;
    int j;

    env->config.simulation_running = 1;
    pthread_mutex_init(&env->config.write_mutex, NULL);
    pthread_mutex_init(&env->config.stop_mutex, NULL);
    pthread_mutex_init(&env->table_mutex, NULL);
    pthread_cond_init(&env->table_cond, NULL);

    env->coders = malloc(sizeof(t_coder) * env->config.nb_coders);
    env->dongles = malloc(sizeof(t_dongle) * env->config.nb_coders);
    if (!env->coders || !env->dongles)
    {
        free(env->coders);
        free(env->dongles);
        env->coders = NULL;
        env->dongles = NULL;
        return (0);
    }
    i = 0;
    while (i < env->config.nb_coders)
    {
        env->coders[i].id = i + 1;
        env->coders[i].compile_count = 0;
        env->coders[i].last_compile_start = 0;
        env->coders[i].env = env;
        pthread_mutex_init(&env->coders[i].state_mutex, NULL);
        i++;
    }

    i = 0;
    while (i < env->config.nb_coders)
    {
        env->dongles[i].id = i + 1;
        env->dongles[i].in_use = 0;
        env->dongles[i].available_at = 0;
        if (!heap_init(&env->dongles[i].wait_queue, env->config.nb_coders))
        {
            j = 0;
            while (j < i)
            {
                heap_free(&env->dongles[j].wait_queue);
                j++;
            }
            return (0);
        }
        i++;
    }

    i = 0;
    while (i < env->config.nb_coders)
    {
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
    {
        i = 0;
        while (i < env->config.nb_coders)
        {
            pthread_mutex_destroy(&env->coders[i].state_mutex);
            i++;
        }
        free(env->coders);
    }
    pthread_mutex_destroy(&env->config.write_mutex);
    pthread_mutex_destroy(&env->config.stop_mutex);
    pthread_mutex_destroy(&env->table_mutex);
    pthread_cond_destroy(&env->table_cond);
}