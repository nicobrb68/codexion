/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nbarbosa <nbarbosa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 09:40:24 by nbarbosa          #+#    #+#             */
/*   Updated: 2026/02/26 15:57:03 by nbarbosa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	arg_filter(int ac, char **av, t_params *params)
{
	if (ac != 9)
		return (1);
	if (!safe_atoi(av[1], &params->number_of_coders)
		|| !safe_atoi(av[2], &params->time_to_burnout)
		|| !safe_atoi(av[3], &params->time_to_compile)
		|| !safe_atoi(av[4], &params->time_to_debug)
		|| !safe_atoi(av[5], &params->time_to_refactor)
		|| !safe_atoi(av[6], &params->number_of_compiles_required)
		|| !safe_atoi(av[7], &params->dongle_cooldown))
		return (1);
	if (params->number_of_coders == 0)
		return (1);
	if (strcmp(av[8], "fifo") == 0)
		params->scheduler = FIFO;
	else if (strcmp(av[8], "edf") == 0)
		params->scheduler = EDF;
	else
		return (1);
	return (0);
}

int main(int ac, char **av)
{
    t_params    params;
    t_hub       hub;

    // 1. On vérifie que le mec a bien tapé ses arguments
    if (arg_filter(ac, av, &params))
    {
        printf("Invalid argument provided, End of program.\n");
        return (1);
    }
    
    // 2. On construit l'usine (les mallocs, les mutex, etc.)
    if (init_hub(&hub, &params))
    {
        printf("Error: Failed to initialize the hub.\n");
        return (1);
    }
    
    // 3. On allume la machine et on attend que ça se termine
    start_simulation(&hub);
    
    // 4. L'usine est fermée, on détruit tout proprement (0 leaks !)
    free_hub(&hub);
    
    return (0);
}
