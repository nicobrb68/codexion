/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbarbosa <nbarbosa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 15:21:40 by nbarbosa          #+#    #+#             */
/*   Updated: 2026/02/26 16:16:12 by nbarbosa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	init_coders(t_hub *hub)
{
	int	i;

	i = 0;
	while (i < hub->params->number_of_coders)
	{
		hub->coders[i].id = i + 1;
		hub->coders[i].hub = hub;
		pthread_mutex_init(&hub->dongles[i].mutex, NULL);
		pthread_mutex_init(&hub->coders[i].coder_mutex, NULL);
		hub->dongles[i].is_used = 0;
		hub->dongles[i].available_at = 0;
		hub->coders[i].compile_count = 0;
		hub->coders[i].last_compile = 0;
		hub->coders[i].left_dongle = &hub->dongles[i];
		hub->coders[i].right_dongle
			= &hub->dongles[(i + 1) % hub->params->number_of_coders];
		i++;
	}
}

static int	init_queue(t_hub *hub)
{
	hub->queue = malloc(sizeof(t_heap));
	if (!hub->queue)
		return (1);
	hub->queue->array = malloc(sizeof(t_coder *)
			* hub->params->number_of_coders);
	if (!hub->queue->array)
	{
		free(hub->queue);
		return (1);
	}
	hub->queue->size = 0;
	return (0);
}

static void	init_mutexes(t_hub *hub)
{
	pthread_mutex_init(&hub->terminal_mutex, NULL);
	pthread_mutex_init(&hub->red_button_mutex, NULL);
	pthread_mutex_init(&hub->heap_mutex, NULL);
	pthread_cond_init(&hub->cond, NULL);
}

int	init_hub(t_hub *hub, t_params *params)
{
	hub->params = params;
	hub->ready = 0;
	hub->finished = 0;
	hub->dongles = malloc(sizeof(t_dongle) * params->number_of_coders);
	if (!hub->dongles)
		return (1);
	hub->coders = malloc(sizeof(t_coder) * params->number_of_coders);
	if (!hub->coders)
	{
		free(hub->dongles);
		return (1);
	}
	if (init_queue(hub))
	{
		free(hub->dongles);
		free(hub->coders);
		return (1);
	}
	init_coders(hub);
	init_mutexes(hub);
	return (0);
}
