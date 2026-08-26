#include "codexion.h"

void *monitor_routine(void *arg)
{
    t_env   *env;
    int     i;
    int     all_completed;
    long    now;
    long    time_since_compile;

    env = (t_env *)arg;
    while (is_simulation_running(&env->config))
    {
        pthread_mutex_lock(&env->table_mutex);
        pthread_cond_broadcast(&env->table_cond); // Réveille les threads si un cooldown s'est écoulé
        pthread_mutex_unlock(&env->table_mutex);

        all_completed = 1;
        i = 0;
        while (i < env->config.nb_coders)
        {
            now = get_time_in_ms();
            time_since_compile = now - env->coders[i].last_compile_start;

            if (time_since_compile >= env->config.time_to_burnout)
            {
                pthread_mutex_lock(&env->config.write_mutex);
                stop_simulation(&env->config);
                printf("%ld %d burned out\n", 
                       now - env->config.start_time, 
                       env->coders[i].id);
                pthread_mutex_unlock(&env->config.write_mutex);

                // Réveil général pour forcer la sortie de tous les threads
                pthread_mutex_lock(&env->table_mutex);
                pthread_cond_broadcast(&env->table_cond);
                pthread_mutex_unlock(&env->table_mutex);
                return (NULL);
            }

            if (env->config.nb_compiles_required == -1 || 
                env->coders[i].compile_count < env->config.nb_compiles_required)
                all_completed = 0;
            i++;
        }

        if (env->config.nb_compiles_required != -1 && all_completed)
        {
            stop_simulation(&env->config);
            pthread_mutex_lock(&env->table_mutex);
            pthread_cond_broadcast(&env->table_cond);
            pthread_mutex_unlock(&env->table_mutex);
            return (NULL);
        }
        usleep(500); // Surveillance haute fréquence (< 1ms)
    }
    return (NULL);
}