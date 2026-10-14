/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   state.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eanass <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 17:00:45 by eanass            #+#    #+#             */
/*   Updated: 2026/09/02 17:00:47 by eanass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	check_stop(t_data *data)
{
	int	stop;

	pthread_mutex_lock(&data->stop_mutex);
	stop = data->stop;
	pthread_mutex_unlock(&data->stop_mutex);
	return (stop);
}

void	safe_stop(t_data *data, int value)
{
	pthread_mutex_lock(&data->stop_mutex);
	data->stop = value;
	pthread_mutex_unlock(&data->stop_mutex);
}

void	safe_print(t_data *data, int id, char *msg)
{
	pthread_mutex_lock(&data->print_mutex);
	pthread_mutex_lock(&data->stop_mutex);
	if (!data->stop)
		printf("%ld %d %s\n", get_time() - data->start_time, id, msg);
	pthread_mutex_unlock(&data->stop_mutex);
	pthread_mutex_unlock(&data->print_mutex);
}

void	safe_increment(t_coder *coder)
{
	pthread_mutex_lock(&coder->data->sched_mutex);
	coder->compile_done++;
	if (coder->data->required_compiles != -1
		&& coder->compile_done == coder->data->required_compiles)
		coder->data->finished_coders++;
	if (coder->data->required_compiles != -1
		&& coder->data->finished_coders == coder->data->number_of_coders)
	{
		pthread_mutex_lock(&coder->data->stop_mutex);
		coder->data->stop = 1;
		pthread_mutex_unlock(&coder->data->stop_mutex);
		pthread_mutex_lock(&coder->data->table_mutex);
		pthread_cond_broadcast(&coder->data->table_cond);
		pthread_mutex_unlock(&coder->data->table_mutex);
	}
	pthread_mutex_unlock(&coder->data->sched_mutex);
}
