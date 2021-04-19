/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 10:41:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/19 11:11:29 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

int	exit_error(char *message)
{
	ft_putstr("Error\n");
	ft_putstr(message);
	return (1);
}

int	clean_exit(t_holder *holder)
{
	(void)holder;
	exit(0);
}
