/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_intdigits_bonus.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/08 12:47:24 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/03/22 14:52:45 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int		ft_intdigits(int n)
{
	int count;

	count = 0;
	if (n == 0)
		return (1);
	if (n < 0)
	{
		if (n == INT_MIN)
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
