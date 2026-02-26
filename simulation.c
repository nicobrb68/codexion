/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbarbosa <nbarbosa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 09:49:34 by nbarbosa          #+#    #+#             */
/*   Updated: 2026/02/26 09:33:26 by nbarbosa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static	int	check_dongles(t_coder *coder)
{
	int			ready;
	long long	now;

	ready = 0;
	now = get_time();
	pthread_mutex_lock(&coder->left_dongle->mutex);
	pthread_mutex_lock(&coder->right_dongle->mutex);
	if (coder->left_dongle->is_used == 0 && coder->right_dongle->is_used == 0
		&& now >= coder->left_dongle->available_at && now >= coder->right_dongle->available_at)
	{
		ready = 1;
	}
	pthread_mutex_unlock(&coder->right_dongle->mutex);
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	return (ready);
}

static void	take_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->hub->heap_mutex);
	coder->request_time = get_time();
	heap_push(coder->hub, coder);
	while (coder->hub->queue->array[0] != coder && !check_dongles(coder))
	{
		pthread_cond_wait(&coder->hub->cond, &coder->hub->heap_mutex);
	}
	heap_pop(coder->hub);
	pthread_mutex_lock(&coder->left_dongle->mutex);
	coder->left_dongle->is_used = 1;
	print_status(coder, "has taken the left dongle");
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	pthread_mutex_lock(&coder->right_dongle->mutex);
	coder->right_dongle->is_used = 1;
	print_status(coder, "has taken the right dongle");
	pthread_mutex_unlock(&coder->right_dongle->mutex);
	pthread_mutex_unlock(&coder->hub->heap_mutex);
}

static int check_all_coders(t_hub *hub, int *finished_count)
{
	int 		i;
	long long	now;

	*finished_count = 0;
	i = -1;
	while (++i < hub->params->number_of_coders)
		{
			now = get_time();
			if (now >= hub->coders[i].last_compile + hub->params->time_to_burnout)
			{
				hub->finished = 1;
				print_status(&hub->coders[i], "burned out");
				return (1);
			}
			if (hub->coders[i].compile_count >= hub->params->number_of_compiles_required)
				(*finished_count)++;
		}
		return (0);
}
		



void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (coder->hub->ready == 0)
		continue ;
	coder->last_compile = coder->hub->start_chrono;
	while (coder->hub->finished == 0)
	{
		take_dongles(coder);
		print_status(coder, "is compiling");
		coder->last_compile = get_time();
		usleep(coder->hub->params->time_to_compile * 1000);
		coder->compile_count++;
		pthread_mutex_unlock(coder->left_dongle);
		pthread_mutex_unlock(coder->right_dongle);
		print_status(coder, "is debugging");
		usleep(coder->hub->params->time_to_debug);
		print_status(coder, "is refactoring");
		usleep(coder->hub->params->time_to_refactor);
	}
	return (NULL);
}

void	*monitor_routine(void * arg)
{
	long long	now;
	int			coder_finished;
	int			i;
	t_hub 		*hub;
	
	coder_finished = 0;
	hub = (t_hub *)arg;

	while (hub->ready != 1)
		continue ;	
	while (coder_finished < hub->params->number_of_coders)
	{
		if (check_all_coders(hub, &coder_finished))
			return (NULL);
		usleep(100);
	}
	hub->finished = 1;
	return (NULL);
}

int	start_simulation(t_hub *hub)
{
	int	i;

	i = -1;
	while (++i < hub->params->number_of_coders)
	{
		hub->coders[i].last_compile = hub->start_chrono;
		pthread_create(&hub->coders[i].thread_id, NULL,
			coder_routine, &hub->coders[i]);
	}
	i = -1;
	pthread_create(&hub->monitor, NULL, monitor_routine, hub);
	while (++i < hub->params->number_of_coders)
	{
		pthread_join(hub->coders[i].thread_id, NULL);
	}
	hub->start_chrono = get_time();
	hub->ready = 1;
	pthread_join(hub->monitor, NULL);
	return (0);
}
