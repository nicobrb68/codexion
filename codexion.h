/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbarbosa <nbarbosa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 10:20:39 by nbarbosa          #+#    #+#             */
/*   Updated: 2026/02/25 17:06:34 by nbarbosa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <string.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <pthread.h>
# include <sys/time.h>

# define FIFO 0
# define EDF  1

typedef struct s_hub	t_hub;
typedef struct s_dongle	t_dongle;

typedef struct s_params
{
	unsigned int	number_of_coders;
	unsigned int	time_to_burnout;
	unsigned int	time_to_compile;
	unsigned int	time_to_debug;
	unsigned int	time_to_refactor;
	unsigned int	number_of_compiles_required;
	unsigned int	dongle_cooldown;
	int				scheduler;
}	t_params;

typedef struct s_coder
{
	unsigned long	id;
	pthread_t		thread_id;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	int				compile_count;
	long long		last_compile;
	t_hub			*hub;
	long long		request_time;
}	t_coder;

typedef struct s_heap
{
	t_coder	**array;
	int		size;
}	t_heap;

typedef struct s_hub
{
	t_params		*params;
	long long		start_chrono;
	t_coder			*coders;
	pthread_t		monitor;
	t_dongle		*dongles;
	int				finished;
	pthread_mutex_t	terminal_mutex;
	pthread_mutex_t	red_button_mutex;
	int				ready;
	t_heap			*queue;
	pthread_cond_t	cond;
	pthread_mutex_t	heap_mutex;
}	t_hub;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	int				is_used;
	long long		available_at;
}	t_dongle;

int			safe_atoi(char *str, unsigned int *nb);
long long	get_time(void);
void		*monitor_routine(void *arg);
void		*coder_routine(void *arg);
void		release_dongles(t_coder *coder);

#endif