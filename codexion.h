/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eanass <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/02 17:02:12 by eanass            #+#    #+#             */
/*   Updated: 2026/09/02 17:02:28 by eanass           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_coder	t_coder;

typedef struct s_data
{
	int				number_of_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				required_compiles;
	int				dongle_cooldown;
	char			*scheduler;
	int				finished_coders;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	sched_mutex;
	pthread_mutex_t	table_mutex;
	pthread_cond_t	table_cond;
	long			start_time;
	int				stop;
	t_coder			*all_coders;
}	t_data;

typedef struct s_dongle
{
	int		id;
	int		in_use;
	long	available_at;
}	t_dongle;

typedef struct s_coder
{
	int			id;
	int			compile_done;
	long		last_compile;
	long		current_priority;
	int			state;
	pthread_t	thread_id;
	t_dongle	*left_dongle;
	t_dongle	*right_dongle;
	t_data		*data;
}	t_coder;

int		parse_args(int ac, char **av, t_data *data);
int		init_simulation(t_data *data, t_coder **coders, t_dongle **dongles);
long	get_time(void);
void	precise_sleep(t_data *data, long time_in_ms);
int		error(char *error_msg);
int		check_stop(t_data *data);
void	safe_stop(t_data *data, int value);
void	safe_print(t_data *data, int id, char *msg);
void	safe_increment(t_coder *coder);
int		create_threads(t_data *data, t_coder *coders, t_dongle *dongles);
int		join_threads(t_data *data, t_coder *coders, t_dongle *dongles);
void	*coder_routine(void *arg);
int		can_take(t_coder *c, long now);
int		is_highest_priority(t_coder *c, long now);
void	*monitor_routine(void *arg);
void	cleanup_simulation(t_data *data, t_coder *coders, t_dongle *dongles);
void	set_coder_priority(t_coder *coder);
int		handle_single_coder(t_coder *coder);

#endif
