/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eanass <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 17:00:13 by eanass            #+#    #+#             */
/*   Updated: 2026/09/02 17:00:15 by eanass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	is_coder_burned_out(t_data *data, t_coder *coder, long now)
{
	if (now - coder->last_compile > (data->time_to_burnout + 5))
	{
		pthread_mutex_lock(&data->print_mutex);
		pthread_mutex_lock(&data->stop_mutex);
		if (!data->stop)
		{
			data->stop = 1;
			printf("%ld %d burned out\n", now - data->start_time, coder->id);
		}
		pthread_mutex_unlock(&data->stop_mutex);
		pthread_mutex_unlock(&data->print_mutex);
		return (1);
	}
	return (0);
}

static int	check_all_coders(t_data *data, t_coder *coders, long now)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		pthread_mutex_lock(&data->sched_mutex);
		if (coders[i].compile_done < data->required_compiles
			&& is_coder_burned_out(data, &coders[i], now))
		{
			pthread_mutex_unlock(&data->sched_mutex);
			return (1);
		}
		pthread_mutex_unlock(&data->sched_mutex);
		i++;
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_coder	*coders;
	t_data	*data;

	coders = (t_coder *)arg;
	data = coders[0].data;
	while (!check_stop(data))
	{
		if (check_all_coders(data, coders, get_time()))
		{
			pthread_mutex_lock(&data->table_mutex);
			pthread_cond_broadcast(&data->table_cond);
			pthread_mutex_unlock(&data->table_mutex);
			return (NULL);
		}
		pthread_mutex_lock(&data->table_mutex);
		pthread_cond_broadcast(&data->table_cond);
		pthread_mutex_unlock(&data->table_mutex);
		usleep(1000);
	}
	return (NULL);
}
