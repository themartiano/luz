/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putint_bonus.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/08 09:58:01 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/08 17:38:42 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	ft_putint_rec(int n, int *bytes, bool sign)
{
	if (n == -2147483648)
	{
		if (sign == true)
			write(1, "-2147483648", 11);
		else
			write(1, "2147483648", 10);
		*bytes = 10 + sign;
		return ;
	}
	if (n < 0)
	{
		if (sign == true)
			*bytes += ft_putchar('-');
		n = -n;
	}
	if (n > 9)
	{
		ft_putint_rec(n / 10, bytes, sign);
		ft_putint_rec(n % 10, bytes, sign);
	}
	else
		*bytes += ft_putchar(n + 48);
}

int	ft_putint(int n, bool sign)
{
	int	bytes;

	bytes = 0;
	ft_putint_rec(n, &bytes, sign);
	return (bytes);
}
