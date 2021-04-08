/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/10 11:31:44 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/03/08 10:04:40 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int			ft_atoi(const char *nptr)
{
	int	sign;
	int	result;

	sign = 1;
	result = -2147483648;
	ft_memcmp(nptr, "-2147483648", 11) != 0 ? (result = 0) : 1;
	while (ft_isspace(*nptr))
		nptr++;
	if (*nptr == '-' || *nptr == '+')
	{
		*nptr == '-' ? (sign = -1) : 1;
		nptr++;
	}
	while (*nptr != '\0')
	{
		if (*nptr >= '0' && *nptr <= '9')
		{
			result *= 10;
			result += (*nptr - 48);
			nptr++;
		}
		else
			return (result * sign);
	}
	return (result * sign);
}
