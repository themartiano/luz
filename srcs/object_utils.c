/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   object_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/26 17:30:29 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/28 13:09:22 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_vec3	random_in_unit(void)
{
	float	f;

	while (true)
	{
		f = 2.0f * drand48() - 1.0f;
		if (f * f + f * f + f * f < 1.0f)
			break ;
	}
	return (set(f, f, f));
}

t_gnrc_obj	*get_gnrc_obj(t_scene *scene)
{
	t_gnrc_obj	*gnrc_obj;

	gnrc_obj = scene->objects->object;
	return (gnrc_obj);
}
