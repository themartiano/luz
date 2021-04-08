/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_longlongdigits_bonus.c                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/29 09:56:48 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/03/29 09:56:55 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int		ft_longlongdigits(long long n)
{
	int count;

	count = 0;
	if (n == 0)
		return (1);
	if (n < 0)
	{
		if (n == LLONG_MIN)
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
