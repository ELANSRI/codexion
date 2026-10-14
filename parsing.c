/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eanass <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 17:00:23 by eanass            #+#    #+#             */
/*   Updated: 2026/09/02 17:00:26 by eanass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	ft_is_number(char *str)
{
	int	i;

	if (!str || !str[0])
		return (0);
	i = 0;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			return (0);
		i++;
	}
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	parse_args(int ac, char **av, t_data *data)
{
	int	i;

	if (ac != 9)
		return (error("Wrong number of arguments"));
	i = 1;
	while (i < 8)
	{
		if (!ft_is_number(av[i]))
			return (error("Arguments must be positive numbers"));
		i++;
	}
	if (strcmp(av[8], "fifo") != 0 && strcmp(av[8], "edf") != 0)
		return (error("Scheduler must be fifo or edf"));
	data->scheduler = av[8];
	data->number_of_coders = atoi(av[1]);
	data->time_to_burnout = atoi(av[2]);
	data->time_to_compile = atoi(av[3]);
	data->time_to_debug = atoi(av[4]);
	data->time_to_refactor = atoi(av[5]);
	data->required_compiles = atoi(av[6]);
	data->dongle_cooldown = atoi(av[7]);
	if (data->number_of_coders <= 0 || data->time_to_burnout <= 0)
		return (error("Invalid values for coders or times"));
	return (1);
}
