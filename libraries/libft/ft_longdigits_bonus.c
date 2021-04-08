/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_longdigits_bonus.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/08 12:47:24 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/03/29 09:57:06 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int		ft_longdigits(long n)
{
	int count;

	count = 0;
	if (n == 0)
		return (1);
	if (n < 0)
	{
		if (n == LONG_MIN)
		{
			n /= 10;
			count++;
			n = -n;
		}
		else
			n = -n;
	}
	while (n > 0)
	{
		n /= 10;
		count++;
	}
	return (count);
}
