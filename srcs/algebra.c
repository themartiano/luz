/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   algebra.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/29 09:01:00 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 09:33:37 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minirt.h"

float	dot(t_vec3 u, t_vec3 v)
{
	return (u.x * v.x + u.y * v.y + u.z * v.z);
}

t_vec3	cross(t_vec3 vec1, t_vec3 vec2)
{
	t_vec3	result;

	result.x = vec1.y * vec2.z - vec1.z * vec2.y;
	result.y = -(vec1.x * vec2.z - vec1.z * vec2.x);
	result.z = vec1.x * vec2.y - vec1.y * vec2.x;
	return (result);
}

t_vec3	mul(t_vec3 vec, float m)
{
	t_vec3	result;

	result.x = vec.x * m;
	result.y = vec.y * m;
	result.z = vec.z * m;
	return (result);
}

t_vec3	divide(t_vec3 vec, float d)
{
	t_vec3	result;

	result.x = vec.x / d;
	result.y = vec.y / d;
	result.z = vec.z / d;
	return (result);
}

t_vec3	random_in_unit(void)
{
	t_vec3	rndm;

	while (true)
	{
		rndm = set(drand48(), drand48(), drand48());
		rndm = mul(rndm, 2.0f);
		rndm = sub(rndm, set(1.0f, 1.0f, 1.0f));
		if (length_sqrt(rndm) < 1.0f)
			break ;
	}
	return (rndm);
}
