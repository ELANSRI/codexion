/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eanass <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 17:00:02 by eanass            #+#    #+#             */
/*   Updated: 2026/09/02 17:00:05 by eanass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_dongles(t_data *data, t_dongle *dongles)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		dongles[i].id = i;
		dongles[i].in_use = 0;
		dongles[i].available_at = 0;
		i++;
	}
	return (1);
}

void	init_coders(t_data *data, t_coder *coders, t_dongle *dongles)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		coders[i].id = i + 1;
		coders[i].compile_done = 0;
		coders[i].last_compile = data->start_time;
		coders[i].current_priority = 0;
		coders[i].state = 0;
		coders[i].data = data;
		coders[i].left_dongle = &dongles[i];
		coders[i].right_dongle = &dongles[(i + 1) % data->number_of_coders];
		i++;
	}
}

int	init_simulation(t_data *data, t_coder **coders, t_dongle **dongles)
{
	*coders = malloc(sizeof(t_coder) * data->number_of_coders);
	*dongles = malloc(sizeof(t_dongle) * data->number_of_coders);
	if (!*coders || !*dongles)
		return (0);
	data->all_coders = *coders;
	data->stop = 0;
	data->finished_coders = 0;
	pthread_mutex_init(&data->print_mutex, NULL);
	pthread_mutex_init(&data->stop_mutex, NULL);
	pthread_mutex_init(&data->sched_mutex, NULL);
	pthread_mutex_init(&data->table_mutex, NULL);
	pthread_cond_init(&data->table_cond, NULL);
	if (!init_dongles(data, *dongles))
		return (0);
	data->start_time = get_time();
	init_coders(data, *coders, *dongles);
	return (1);
}
