/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbarbosa <nbarbosa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 13:23:36 by nbarbosa          #+#    #+#             */
/*   Updated: 2026/02/25 17:32:29 by nbarbosa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	priority(t_coder *a, t_coder *b, t_params *params)
{
	long long	deadline_a;
	long long	deadline_b;

	if (params->scheduler == FIFO)
		return (a->request_time < b->request_time);
	deadline_a = a->last_compile + params->time_to_burnout;
	deadline_b = b->last_compile + params->time_to_burnout;
	if (deadline_a != deadline_b)
		return (deadline_a < deadline_b);
	return (a->request_time < b->request_time);
}

void	release_dongles(t_coder *coder)
{
	long long	now;

	now = get_time();
	pthread_mutex_lock(&coder->left_dongle->mutex);
	coder->left_dongle->is_used = 0;
	coder->left_dongle->available_at = now + coder->hub->params->dongle_cooldown;
	pthread_mutex_unlock(&coder->left_dongle->mutex);
	now = get_time();
	pthread_mutex_lock(&coder->right_dongle->mutex);
	coder->right_dongle->is_used = 0;
	coder->right_dongle->available_at = now + coder->hub->params->dongle_cooldown;
	pthread_mutex_unlock(&coder->right_dongle->mutex);
	pthread_mutex_lock(&coder->hub->heap_mutex);
	pthread_cond_broadcast(&coder->hub->cond);
	pthread_mutex_unlock(&coder->hub->heap_mutex);
}

void	put_down(t_hub *hub, int index)
{
	int	index_left;
	int	index_right;
	int	boss_index;

	while (1)
	{
		boss_index = index;
		index_left = (2 * boss_index) + 1;
		index_right = (2 * boss_index) + 2;
		if (index_left < hub->queue->size
			&& priority(hub->queue->array[index_left],
				hub->queue->array[boss_index], hub->params))
			boss_index = index_left;
		if (index_right < hub->queue->size
			&& priority(hub->queue->array[index_right],
				hub->queue->array[boss_index], hub->params))
			boss_index = index_right;
		if (boss_index != index)
		{
			swap_coders(&hub->queue->array[index],
				&hub->queue->array[boss_index]);
			index = boss_index;
		}
		else
			break ;
	}
}

void	heap_push(t_hub *hub, t_coder *new_coder)
{
	int	index;
	int	parent_index;

	index = hub->queue->size;
	hub->queue->array[index] = new_coder;
	hub->queue->size++;
	while (index > 0)
	{
		parent_index = (index -1) / 2;
		if (priority(hub->queue->array[index], hub->queue->array[parent_index],
				hub->params))
		{
			swap_coders(&hub->queue->array[index],
				&hub->queue->array[parent_index]);
			index = parent_index;
		}
		else
			break ;
	}
}

t_coder	*heap_pop(t_hub *hub)
{
	t_coder	*boss;

	if (hub->queue->size == 0)
		return (NULL);
	boss = hub->queue->array[0];
	hub->queue->size--;
	if (hub->queue->size > 0)
	{
		hub->queue->array[0] = hub->queue->array[hub->queue->size];
		put_down(hub, 0);
	}
	return (boss);
}
