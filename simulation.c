/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbarbosa <nbarbosa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 09:49:34 by nbarbosa          #+#    #+#             */
/*   Updated: 2026/02/26 16:42:10 by nbarbosa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	check_dongles(t_coder *coder)
{
	int			ready;
	t_dongle	*first;
	t_dongle	*second;

	ready = 0;
	first = coder->left_dongle;
	second = coder->right_dongle;
	if (first > second)
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	if (coder->left_dongle->is_used == 0 && coder->right_dongle->is_used == 0)
	{
		if (get_time() >= coder->left_dongle->available_at
			&& get_time() >= coder->right_dongle->available_at)
			ready = 1;
		else
			ready = 2;
	}
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&first->mutex);
	return (ready);
}

static int	wait_dongles(t_coder *coder)
{
	int	status;

	while (check_if_finished(coder->hub) == 0)
	{
		if (coder->hub->queue->array[0] == coder)
		{
			status = check_dongles(coder);
			if (status == 1)
				return (1);
			if (status == 2)
			{
				pthread_mutex_unlock(&coder->hub->heap_mutex);
				usleep(1000);
				pthread_mutex_lock(&coder->hub->heap_mutex);
				continue ;
			}
		}
		pthread_cond_wait(&coder->hub->cond, &coder->hub->heap_mutex);
	}
	return (0);
}

static void	take_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->hub->heap_mutex);
	coder->request_time = get_time();
	heap_push(coder->hub, coder);
	if (!wait_dongles(coder))
	{
		pthread_mutex_unlock(&coder->hub->heap_mutex);
		return ;
	}
	heap_pop(coder->hub);
	pthread_mutex_lock(&coder->left_dongle->mutex);
	coder->left_dongle->is_used = 1;
	print_status(coder, "has taken a dongle");
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	pthread_mutex_lock(&coder->right_dongle->mutex);
	coder->right_dongle->is_used = 1;
	print_status(coder, "has taken a dongle");
	pthread_mutex_unlock(&coder->right_dongle->mutex);
	pthread_mutex_unlock(&coder->hub->heap_mutex);
}

static void	*solo_routine(t_coder *coder)
{
	pthread_mutex_lock(&coder->left_dongle->mutex);
	print_status(coder, "has taken a dongle");
	while (check_if_finished(coder->hub) == 0)
		usleep(1000);
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	return (NULL);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (check_if_ready(coder->hub) == 0)
		usleep(1000);
	if (coder->hub->params->number_of_coders == 1)
		return (solo_routine(coder));
	while (check_if_finished(coder->hub) == 0)
	{
		take_dongles(coder);
		print_status(coder, "is compiling");
		pthread_mutex_lock(&coder->coder_mutex);
		coder->last_compile = get_time();
		pthread_mutex_unlock(&coder->coder_mutex);
		usleep(coder->hub->params->time_to_compile * 1000);
		pthread_mutex_lock(&coder->coder_mutex);
		coder->compile_count++;
		pthread_mutex_unlock(&coder->coder_mutex);
		release_dongles(coder);
		print_status(coder, "is debugging");
		usleep(coder->hub->params->time_to_debug * 1000);
		print_status(coder, "is refactoring");
		usleep(coder->hub->params->time_to_refactor * 1000);
	}
	return (NULL);
}
