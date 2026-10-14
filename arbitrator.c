/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arbitrator.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eanass <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 17:00:34 by eanass            #+#    #+#             */
/*   Updated: 2026/09/02 17:00:37 by eanass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	can_take(t_coder *c, long now)
{
	if (c->left_dongle->in_use || now < c->left_dongle->available_at)
		return (0);
	if (c->right_dongle->in_use || now < c->right_dongle->available_at)
		return (0);
	return (1);
}

int	is_highest_priority(t_coder *c, long now)
{
	int		i;
	t_coder	*other;

	i = 0;
	while (i < c->data->number_of_coders)
	{
		other = &c->data->all_coders[i];
		if (other->id != c->id && other->state == 1)
		{
			if (other->left_dongle == c->left_dongle
				|| other->right_dongle == c->right_dongle
				|| other->left_dongle == c->right_dongle
				|| other->right_dongle == c->left_dongle)
			{
				if (other->current_priority < c->current_priority
					|| (other->current_priority == c->current_priority
						&& other->id < c->id))
					if (can_take(other, now))
						return (0);
			}
		}
		i++;
	}
	return (1);
}

void	set_coder_priority(t_coder *coder)
{
	pthread_mutex_lock(&coder->data->sched_mutex);
	if (strcmp(coder->data->scheduler, "edf") == 0)
		coder->current_priority = coder->last_compile
			+ coder->data->time_to_burnout;
	else
		coder->current_priority = get_time();
	pthread_mutex_unlock(&coder->data->sched_mutex);
}

int	handle_single_coder(t_coder *coder)
{
	if (coder->data->number_of_coders == 1)
	{
		safe_print(coder->data, coder->id, "has taken a dongle");
		while (!check_stop(coder->data))
			precise_sleep(coder->data, 10);
		return (1);
	}
	return (0);
}
