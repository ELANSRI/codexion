/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eanass <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 16:59:52 by eanass            #+#    #+#             */
/*   Updated: 2026/09/02 16:59:54 by eanass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	cleanup_simulation(t_data *data, t_coder *coders, t_dongle *dongles)
{
	if (dongles)
		free(dongles);
	pthread_mutex_destroy(&data->print_mutex);
	pthread_mutex_destroy(&data->stop_mutex);
	pthread_mutex_destroy(&data->sched_mutex);
	pthread_mutex_destroy(&data->table_mutex);
	pthread_cond_destroy(&data->table_cond);
	if (coders)
		free(coders);
}

int	main(int ac, char **av)
{
	t_data		data;
	t_coder		*coders;
	t_dongle	*dongles;
	pthread_t	monitor_id;

	if (!parse_args(ac, av, &data))
		return (1);
	if (!init_simulation(&data, &coders, &dongles))
		return (1);
	if (!create_threads(&data, coders, dongles))
		return (1);
	if (pthread_create(&monitor_id, NULL, monitor_routine, coders) != 0)
	{
		safe_stop(&data, 1);
		join_threads(&data, coders, dongles);
		return (1);
	}
	pthread_join(monitor_id, NULL);
	if (!join_threads(&data, coders, dongles))
		return (1);
	return (0);
}
