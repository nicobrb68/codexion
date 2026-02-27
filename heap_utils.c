/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbarbosa <nbarbosa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:30:55 by nbarbosa          #+#    #+#             */
/*   Updated: 2026/02/26 16:17:21 by nbarbosa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	swap_coders(t_coder **a, t_coder **b)
{
	t_coder	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	free_hub(t_hub *hub)
{
	int	i;

	i = -1;

	while (++i < hub->params->number_of_coders)
	{
		pthread_mutex_destroy(&hub->dongles[i].mutex);
		pthread_mutex_destroy(&hub->coders[i].coder_mutex);
	}
	pthread_mutex_destroy(&hub->heap_mutex);
	pthread_mutex_destroy(&hub->red_button_mutex);
	pthread_mutex_destroy(&hub->terminal_mutex);
	pthread_cond_destroy(&hub->cond);
	free(hub->queue->array);
	free(hub->queue);
    free(hub->coders);
    free(hub->dongles);
}

int check_if_finished(t_hub *hub)
{
	int	status;

	pthread_mutex_lock(&hub->red_button_mutex);
	status = hub->finished;
	pthread_mutex_unlock(&hub->red_button_mutex);   
	return (status);
}

int check_if_ready(t_hub *hub)
{
    int status;

    pthread_mutex_lock(&hub->red_button_mutex);
    status = hub->ready;
    pthread_mutex_unlock(&hub->red_button_mutex);
    return (status);
}
