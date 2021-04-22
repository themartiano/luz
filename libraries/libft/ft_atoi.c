/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/10 11:31:44 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/08 16:34:11 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_atoi(const char *nptr)
{
	int	sign;
	int	result;

	sign = 1;
	if (ft_memcmp(nptr, "-2147483648", 11) != 0)
		result = 0;
	else
		result = -2147483648;
	while (ft_isspace(*nptr))
		nptr++;
	if (*nptr == '-' || *nptr == '+')
		if (*nptr++ == '-')
			sign = -1;
	while (*nptr != '\0')
	{
		if (ft_isdigit(*nptr))
			result = result * 10 + *nptr++ - 48;
		else
			return (result * sign);
	}
	return (result * sign);
}
