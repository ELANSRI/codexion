#include "codexion.h"

static void	set_wakeup_deadline(struct timespec *ts)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_nsec += 1000000;
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec += 1;
		ts->tv_nsec -= 1000000000;
	}
}

void *monitor_routine(void *arg)
{
	t_env			*env;
	int				i;
	int				all_completed;
	long			now;
	long			last_compile_start;
	int				compile_count;
	struct timespec	wakeup_at;

	env = (t_env *)arg;
	while (is_simulation_running(&env->config))
	{
		pthread_mutex_lock(&env->table_mutex);
		set_wakeup_deadline(&wakeup_at);
		pthread_cond_timedwait(&env->table_cond, &env->table_mutex, &wakeup_at);
		pthread_cond_broadcast(&env->table_cond);
		pthread_mutex_unlock(&env->table_mutex);

		all_completed = 1;
		i = 0;
		while (i < env->config.nb_coders)
		{
			pthread_mutex_lock(&env->coders[i].state_mutex);
			last_compile_start = env->coders[i].last_compile_start;
			compile_count = env->coders[i].compile_count;
			pthread_mutex_unlock(&env->coders[i].state_mutex);
			now = get_time_in_ms();
			if ((now - last_compile_start) >= env->config.time_to_burnout)
			{
				pthread_mutex_lock(&env->config.stop_mutex);
				if (env->config.simulation_running)
				{
					env->config.simulation_running = 0;
					pthread_mutex_lock(&env->config.write_mutex);
					printf("%ld %d burned out\n", now - env->config.start_time,
						env->coders[i].id);
					pthread_mutex_unlock(&env->config.write_mutex);
				}
				pthread_mutex_unlock(&env->config.stop_mutex);
				pthread_mutex_lock(&env->table_mutex);
				pthread_cond_broadcast(&env->table_cond);
				pthread_mutex_unlock(&env->table_mutex);
				return (NULL);
			}
			if (env->config.nb_compiles_required == -1
				|| compile_count < env->config.nb_compiles_required)
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
	}
	return (NULL);
}