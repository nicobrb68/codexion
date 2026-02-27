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

static	int	check_dongles(t_coder *coder)
{
	int			ready;
	long long	now;

	ready = 0;
	now = get_time();
	pthread_mutex_lock(&coder->left_dongle->mutex);
	pthread_mutex_lock(&coder->right_dongle->mutex);
	if (coder->left_dongle->is_used == 0 && coder->right_dongle->is_used == 0)
	{
		if (now >= coder->left_dongle->available_at && now >= coder->right_dongle->available_at)
			ready = 1;
		else
			ready = 2;
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
	while (check_if_finished(coder->hub) == 0)
    {
        if (coder->hub->queue->array[0] == coder)
        {
            int status = check_dongles(coder);
            if (status == 1)
                break;
            if (status == 2)
            {
                pthread_mutex_unlock(&coder->hub->heap_mutex);
                usleep(1000); 
                pthread_mutex_lock(&coder->hub->heap_mutex);
                continue;
            }
        }
        pthread_cond_wait(&coder->hub->cond, &coder->hub->heap_mutex);
    }
    if (check_if_finished(coder->hub))
    {
        pthread_mutex_unlock(&coder->hub->heap_mutex);
        return;
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

static int check_all_coders(t_hub *hub, int *finished_count)
{
	int 		i;
	long long	now;
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
			now = get_time();
			if (now >= last + hub->params->time_to_burnout)
			{
				print_status(&hub->coders[i], "burned out");
				pthread_mutex_lock(&hub->red_button_mutex);
				hub->finished = 1;
				pthread_mutex_unlock(&hub->red_button_mutex);
				pthread_mutex_lock(&hub->heap_mutex);
				pthread_cond_broadcast(&hub->cond);
				pthread_mutex_unlock(&hub->heap_mutex);
				return (1);
			}
			if (count >= hub->params->number_of_compiles_required)
				(*finished_count)++;
		}
		return (0);
}

static void	*solo_routine(t_coder *coder)
{
	pthread_mutex_lock(&coder->left_dongle->mutex);
	print_status(coder, "has taken a dongle");
	while (check_if_finished(coder->hub) == 0)
		continue ;
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	return (NULL);
}


void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (check_if_ready(coder->hub))
		usleep(1000) ;
	pthread_mutex_lock(&coder->coder_mutex);
    coder->last_compile = coder->hub->start_chrono;
    pthread_mutex_unlock(&coder->coder_mutex);
	if (coder->hub->params->number_of_coders == 1)
		return (solo_routine(coder));
	while (check_if_finished(coder->hub) == 0)
	{
		take_dongles(coder);
		print_status(coder, "is compiling");
		pthread_mutex_lock(&coder->coder_mutex);
		coder->last_compile = get_time();
		usleep(coder->hub->params->time_to_compile * 1000);
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

void	*monitor_routine(void * arg)
{
	int			coder_finished;
	t_hub 		*hub;
	
	coder_finished = 0;
	hub = (t_hub *)arg;

	while (check_if_ready(hub))
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

int start_simulation(t_hub *hub)
{
    int i;

    i = -1;
    while (++i < hub->params->number_of_coders)
    {
        pthread_create(&hub->coders[i].thread_id, NULL, coder_routine, &hub->coders[i]);
    }
    pthread_create(&hub->monitor, NULL, monitor_routine, hub);
    hub->start_chrono = get_time();
    i = -1;
    while (++i < hub->params->number_of_coders)
    {
        hub->coders[i].last_compile = hub->start_chrono; 
    }
	pthread_mutex_lock(&hub->red_button_mutex);
    hub->ready = 1;
    pthread_mutex_unlock(&hub->red_button_mutex);
    i = -1;
    while (++i < hub->params->number_of_coders)
    {
        pthread_join(hub->coders[i].thread_id, NULL);
    }
    pthread_join(hub->monitor, NULL);
    return (0);
}
