/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbarbosa <nbarbosa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 15:21:40 by nbarbosa          #+#    #+#             */
/*   Updated: 2026/02/24 16:23:22 by nbarbosa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	kill_coder(t_hub *hub, int i)
{
	pthread_mutex_lock(&hub->terminal_mutex);
	pthread_mutex_lock(&hub->red_button_mutex);
	hub->finished = 1;
	pthread_mutex_unlock(&hub->red_button_mutex);
	printf("%lld %d %s\n", get_time() - hub->start_chrono,
		hub->coders[i].id, "burned out");
	pthread_mutex_unlock(&hub->terminal_mutex);
	pthread_mutex_lock(&hub->heap_mutex);
	pthread_cond_broadcast(&hub->cond);
	pthread_mutex_unlock(&hub->heap_mutex);
	return (1);
}

static int	check_all_coders(t_hub *hub, int *finished_count)
{
	int			i;
	long long	last;
	int			count;

	*finished_count = 0;
	i = -1;
	while (++i < hub->params->number_of_coders)
	{
		pthread_mutex_lock(&hub->coders[i].coder_mutex);
		last = hub->coders[i].last_compile;
		count = hub->coders[i].compile_count;
		pthread_mutex_unlock(&hub->coders[i].coder_mutex);
		if (get_time() >= last + hub->params->time_to_burnout)
			return (kill_coder(hub, i));
		if (count >= hub->params->number_of_compiles_required)
			(*finished_count)++;
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	int		coder_finished;
	t_hub	*hub;

	coder_finished = 0;
	hub = (t_hub *)arg;
	while (check_if_ready(hub) == 0)
		usleep(50);
	while (coder_finished < hub->params->number_of_coders)
	{
		if (check_all_coders(hub, &coder_finished))
			return (NULL);
		usleep(100);
	}
	pthread_mutex_lock(&hub->red_button_mutex);
	hub->finished = 1;
	pthread_mutex_unlock(&hub->red_button_mutex);
	pthread_mutex_lock(&hub->heap_mutex);
	pthread_cond_broadcast(&hub->cond);
	pthread_mutex_unlock(&hub->heap_mutex);
	return (NULL);
}

int	start_simulation(t_hub *hub)
{
	int	i;

	i = -1;
	while (++i < hub->params->number_of_coders)
		pthread_create(&hub->coders[i].thread_id, NULL,
			coder_routine, &hub->coders[i]);
	pthread_create(&hub->monitor, NULL, monitor_routine, hub);
	pthread_mutex_lock(&hub->red_button_mutex);
	hub->start_chrono = get_time();
	hub->ready = 1;
	i = -1;
	while (++i < hub->params->number_of_coders)
	{
		pthread_mutex_lock(&hub->coders[i].coder_mutex);
		hub->coders[i].last_compile = hub->start_chrono;
		pthread_mutex_unlock(&hub->coders[i].coder_mutex);
	}
	pthread_mutex_unlock(&hub->red_button_mutex);
	i = -1;
	while (++i < hub->params->number_of_coders)
		pthread_join(hub->coders[i].thread_id, NULL);
	pthread_join(hub->monitor, NULL);
	return (0);
}
