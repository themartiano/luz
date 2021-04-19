/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putuint_bonus.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/08 09:58:01 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/08 17:39:05 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static void	ft_putuint_rec(unsigned int n, int *bytes)
{
	if (n > 9)
	{
		ft_putuint_rec(n / 10, bytes);
		ft_putuint_rec(n % 10, bytes);
	}
	else
	{
		*bytes += ft_putchar(n + 48);
	}
}

int	ft_putuint(unsigned int n)
{
	int	bytes;

	bytes = 0;
	ft_putuint_rec(n, &bytes);
	return (bytes);
}
