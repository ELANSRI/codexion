/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eanass <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 17:00:55 by eanass            #+#    #+#             */
/*   Updated: 2026/09/02 17:00:56 by eanass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	create_threads(t_data *data, t_coder *coders, t_dongle *dongles)
{
	int	i;

	i = 0;
	while (i < data->number_of_coders)
	{
		if (pthread_create(&coders[i].thread_id, NULL,
				&coder_routine, &coders[i]) != 0)
		{
			safe_stop(data, 1);
			while (--i >= 0)
				pthread_join(coders[i].thread_id, NULL);
			cleanup_simulation(data, coders, dongles);
			return (0);
		}
		i++;
	}
	return (1);
}

int	join_threads(t_data *data, t_coder *coders, t_dongle *dongles)
{
	int	i;
	int	safe;

	safe = 1;
	i = 0;
	while (i < data->number_of_coders)
	{
		if (pthread_join(coders[i].thread_id, NULL) != 0)
			safe = 0;
		i++;
	}
	cleanup_simulation(data, coders, dongles);
	return (safe == 1);
}
