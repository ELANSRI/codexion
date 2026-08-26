#include "codexion.h"

long get_time_in_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000L) + (tv.tv_usec / 1000L));
}

void precise_sleep(long time_in_ms)
{
    long start;

    start = get_time_in_ms();
    while ((get_time_in_ms() - start) < time_in_ms)
        usleep(250);
}

int is_simulation_running(t_config *config)
{
    int running;

    pthread_mutex_lock(&config->stop_mutex);
    running = config->simulation_running;
    pthread_mutex_unlock(&config->stop_mutex);
    return (running);
}

void stop_simulation(t_config *config)
{
    pthread_mutex_lock(&config->stop_mutex);
    config->simulation_running = 0;
    pthread_mutex_unlock(&config->stop_mutex);
}

void print_status(t_coder *coder, const char *status)
{
    long current_time;

    if (!is_simulation_running(&coder->env->config))
        return;
    pthread_mutex_lock(&coder->env->config.write_mutex);
    if (is_simulation_running(&coder->env->config))
    {
        current_time = get_time_in_ms() - coder->env->config.start_time;
        printf("%ld %d %s\n", current_time, coder->id, status);
    }
    pthread_mutex_unlock(&coder->env->config.write_mutex);
}