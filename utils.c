/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbarbosa <nbarbosa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 09:40:14 by nbarbosa          #+#    #+#             */
/*   Updated: 2026/02/25 15:53:26 by nbarbosa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_digit(char *str)
{
	int	i;
	int	has_digit;

	i = 0;
	has_digit = 0;
	if (!str || !str[0])
		return (0);
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '+')
		i++;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		has_digit = 1;
		i++;
	}
	return (has_digit);
}

static long long	ft_atol(char *str)
{
	int			i;
	long long	result;
	long long	sign;

	result = 0;
	sign = 1;
	i = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] && str[i] >= '0' && str[i] <= '9')
	{
		result = result * 10;
		result = result + str[i] - '0';
		if (result > 2147483647)
			return (2147483648);
		i++;
	}
	return (result * sign);
}

int	safe_atoi(char *str, unsigned int *nb)
{
	long long	value;

	if (!is_digit(str))
		return (0);
	value = ft_atol(str);
	if (value > 2147483647 || value < 0)
		return (0);
	*nb = value;
	return (1);
}

long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (((long long)tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	print_status(t_coder *coder, char *msg)
{
	long long	timestamp;

	pthread_mutex_lock(&coder->hub->terminal_mutex);
	timestamp = get_time() - coder->hub->start_chrono;
	if (coder->hub->finished == 0)
	{
		printf("%lld %d %s\n", timestamp, coder->id, msg);
	}
	pthread_mutex_unlock(&coder->hub->terminal_mutex);
}
