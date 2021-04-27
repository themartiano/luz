/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/19 10:41:02 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/27 11:46:43 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

int	exit_error(char *message)
{
	ft_putstr(COLOR_RED "Error\n" COLOR_NC);
	ft_putstr(message);
	exit(1);
}

int	clean_exit(t_scene *scene)
{
	(void)scene;
	exit(0);
}
