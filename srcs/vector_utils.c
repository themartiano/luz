/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/21 11:08:33 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 17:09:30 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

t_vec3	set(float x, float y, float z)
{
	t_vec3	result;

	result.x = x;
	result.y = y;
	result.z = z;
	return (result);
}

t_vec3	sum(t_vec3 vec1, t_vec3 vec2)
{
	t_vec3	result;

	result.x = vec1.x + vec2.x;
	result.y = vec1.y + vec2.y;
	result.z = vec1.z + vec2.z;
	return (result);
}

t_vec3	sub(t_vec3 vec1, t_vec3 vec2)
{
	t_vec3	result;

	result.x = vec1.x - vec2.x;
	result.y = vec1.y - vec2.y;
	result.z = vec1.z - vec2.z;
	return (result);
}

t_vec3	unit_vector(t_vec3 vector)
{
	t_vec3	result;
	float	k;

	k = 1.0f / sqrt(vector.x * vector.x + vector.y * vector.y
			+ vector.z * vector.z);
	result.x = vector.x * k;
	result.y = vector.y * k;
	result.z = vector.z * k;
	return (result);
}

t_vec3	normalize(t_vec3 vector)
{
	float	w;

	w = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	vector.x /= w;
	vector.y /= w;
	vector.z /= w;
	return (vector);
}
