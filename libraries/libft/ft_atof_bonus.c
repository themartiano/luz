/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atof_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/22 08:46:20 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/22 09:44:42 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <math.h>
#include "libft.h"

float	ft_atof(const char *nptr)
{
	int	integer;
	int	decimal;
	float	f_decimal;
	int	sign;

	sign = 1;
	while (ft_isspace(*nptr))
		nptr++;
	if (*nptr == '-' || *nptr == '+')
		if (*nptr++ == '-')
			sign = -1;
	while (ft_isdigit(*nptr))
		integer = integer * 10 + *nptr++ - 48;
	if (*nptr == '.')
		nptr++;
	while (ft_isdigit(*nptr))
		decimal = decimal * 10 + *nptr++ - 48;
	int temp = pow(10, ft_intdigits(decimal));
	f_decimal = decimal / (float)temp;
	return ((float)integer + f_decimal);
}
