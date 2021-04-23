/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atof_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/22 08:46:20 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/23 09:19:46 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <float.h>
#include <math.h>
#include "libft.h"

float	ft_atof(const char *nptr)
{
	float	integer;
	float	decimal;
	float	f_decimal;
	float	sign;

	integer = 0.0f;
	decimal = 0.0f;
	sign = 1.0f;
	while (ft_isspace(*nptr))
		nptr++;
	if (*nptr == '-' || *nptr == '+')
		if (*nptr++ == '-')
			sign = -1.0f;
	while (*nptr && ft_isdigit(*nptr))
		integer = integer * 10.0f + *nptr++ - 48;
	if (*nptr && *nptr == '.')
		nptr++;
	while (*nptr && ft_isdigit(*nptr))
		decimal = decimal * 10.0f + *nptr++ - 48;
	f_decimal = decimal / pow(10, ft_intdigits(decimal));
	return ((integer + f_decimal) * sign);
}
