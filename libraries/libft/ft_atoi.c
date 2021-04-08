/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/10 11:31:44 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/08 16:34:11 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	check_min_val(const char *nptr)
{
	int	result;

	if (ft_memcmp(nptr, "-2147483648", 11) != 0)
		result = 0;
	else
		result = -2147483648;
	return (result);
}

int	ft_atoi(const char *nptr)
{
	int	sign;
	int	result;

	sign = 1;
	result = check_min_val(nptr);
	while (ft_isspace(*nptr))
		nptr++;
	if (*nptr == '-' || *nptr == '+')
	{
		if (*nptr == '-')
			sign = -1;
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
