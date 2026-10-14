/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eanass <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 17:00:34 by eanass            #+#    #+#             */
/*   Updated: 2026/09/02 17:00:37 by eanass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	safe_print_taken(t_coder *coder)
{
	pthread_mutex_lock(&coder->data->print_mutex);
	pthread_mutex_lock(&coder->data->stop_mutex);
	if (!coder->data->stop)
	{
		printf("%ld %d has taken a dongle\n",
			get_time() - coder->data->start_time, coder->id);
		printf("%ld %d has taken a dongle\n",
			get_time() - coder->data->start_time, coder->id);
		printf("%ld %d is compiling\n",
			get_time() - coder->data->start_time, coder->id);
	}
	pthread_mutex_unlock(&coder->data->stop_mutex);
	pthread_mutex_unlock(&coder->data->print_mutex);
}

static void	take_dongles(t_coder *coder)
{
	if (handle_single_coder(coder))
		return ;
	set_coder_priority(coder);
	pthread_mutex_lock(&coder->data->table_mutex);
	coder->state = 1;
	while (!check_stop(coder->data))
	{
		if (can_take(coder, get_time())
			&& is_highest_priority(coder, get_time()))
		{
			coder->left_dongle->in_use = 1;
			coder->right_dongle->in_use = 1;
			coder->state = 0;
			safe_print_taken(coder);
			break ;
		}
		pthread_cond_wait(&coder->data->table_cond, &coder->data->table_mutex);
	}
	if (check_stop(coder->data))
		coder->state = 0;
	pthread_mutex_unlock(&coder->data->table_mutex);
}

static void	release_dongles(t_coder *coder)
{
	long	cooldown_end;

	cooldown_end = get_time() + coder->data->dongle_cooldown;
	pthread_mutex_lock(&coder->data->table_mutex);
	coder->left_dongle->in_use = 0;
	coder->left_dongle->available_at = cooldown_end;
	coder->right_dongle->in_use = 0;
	coder->right_dongle->available_at = cooldown_end;
	pthread_cond_broadcast(&coder->data->table_cond);
	pthread_mutex_unlock(&coder->data->table_mutex);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!check_stop(coder->data))
	{
		take_dongles(coder);
		if (check_stop(coder->data))
			break ;
		pthread_mutex_lock(&coder->data->sched_mutex);
		coder->last_compile = get_time();
		pthread_mutex_unlock(&coder->data->sched_mutex);
		precise_sleep(coder->data, coder->data->time_to_compile);
		release_dongles(coder);
		safe_increment(coder);
		safe_print(coder->data, coder->id, "is debugging");
		precise_sleep(coder->data, coder->data->time_to_debug);
		safe_print(coder->data, coder->id, "is refactoring");
		precise_sleep(coder->data, coder->data->time_to_refactor);
	}
	return (NULL);
}
