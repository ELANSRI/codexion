#include "codexion.h"

static int is_dongle_ready(t_dongle *d, int coder_id, long now)
{
    t_req top;

    if (d->in_use)
        return (0);
    if (now < d->available_at)
        return (0);
    if (heap_peek(&d->wait_queue, &top) && top.coder_id == coder_id)
        return (1);
    return (0);
}

void acquire_dongles(t_coder *coder)
{
    t_req   req;
    long    now;
    long    last_compile_start;
    t_env   *env;

    env = coder->env;
    now = get_time_in_ms();
    
    // Configuration de la priorité selon le scheduler
    req.coder_id = coder->id;
    req.arrival_time = now;
    if (env->config.scheduler == FIFO)
        req.priority_key = now;
    else
    {
        pthread_mutex_lock(&coder->state_mutex);
        last_compile_start = coder->last_compile_start;
        pthread_mutex_unlock(&coder->state_mutex);
        req.priority_key = last_compile_start + env->config.time_to_burnout;
    }

    pthread_mutex_lock(&env->table_mutex);

    // Cas d'un codeur unique : un seul dongle disponible sur la table
    if (coder->left_dongle == coder->right_dongle)
    {
        heap_push(&coder->left_dongle->wait_queue, req, env->config.scheduler);
        while (is_simulation_running(&env->config))
        {
            now = get_time_in_ms();
            if (is_dongle_ready(coder->left_dongle, coder->id, now))
            {
                coder->left_dongle->in_use = 1;
                heap_remove_id(&coder->left_dongle->wait_queue, coder->id,
                    env->config.scheduler);
                pthread_mutex_unlock(&env->table_mutex);
                if (is_simulation_running(&env->config))
                    print_status(coder, "has taken a dongle");
                pthread_mutex_lock(&env->table_mutex);
                while (is_simulation_running(&env->config))
                    pthread_cond_wait(&env->table_cond, &env->table_mutex);
                coder->left_dongle->in_use = 0;
                coder->left_dongle->available_at = get_time_in_ms()
                    + env->config.dongle_cooldown;
                pthread_cond_broadcast(&env->table_cond);
                pthread_mutex_unlock(&env->table_mutex);
                return ;
            }
            pthread_cond_wait(&env->table_cond, &env->table_mutex);
        }
        heap_remove_id(&coder->left_dongle->wait_queue, coder->id, env->config.scheduler);
        pthread_mutex_unlock(&env->table_mutex);
        return;
    }

    // Ajout de la requête aux files des deux dongles convoités
    heap_push(&coder->left_dongle->wait_queue, req, env->config.scheduler);
    heap_push(&coder->right_dongle->wait_queue, req, env->config.scheduler);

    while (is_simulation_running(&env->config))
    {
        now = get_time_in_ms();
        if (is_dongle_ready(coder->left_dongle, coder->id, now) &&
            is_dongle_ready(coder->right_dongle, coder->id, now))
        {
            // Attribution des deux dongles
            coder->left_dongle->in_use = 1;
            coder->right_dongle->in_use = 1;
            heap_remove_id(&coder->left_dongle->wait_queue, coder->id, env->config.scheduler);
            heap_remove_id(&coder->right_dongle->wait_queue, coder->id, env->config.scheduler);
            break;
        }
        pthread_cond_wait(&env->table_cond, &env->table_mutex);
    }
    pthread_mutex_unlock(&env->table_mutex);

    if (is_simulation_running(&env->config))
        print_status(coder, "has taken a dongle");
}

void release_dongles(t_coder *coder)
{
    t_env   *env;
    long    cooldown_end;

    env = coder->env;
    cooldown_end = get_time_in_ms() + env->config.dongle_cooldown;

    pthread_mutex_lock(&env->table_mutex);
    coder->left_dongle->in_use = 0;
    coder->left_dongle->available_at = cooldown_end;
    coder->right_dongle->in_use = 0;
    coder->right_dongle->available_at = cooldown_end;

    // Réveil de tous les threads en attente
    pthread_cond_broadcast(&env->table_cond);
    pthread_mutex_unlock(&env->table_mutex);
}