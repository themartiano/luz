/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnstr_bonus.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/08 09:56:38 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 16:00:15 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_putnstr(const char *str, int n)
{
	int		bytes;
	int		i;
	char	c;

	i = 0;
	bytes = 0;
	while (*(str + i) != '\0' && i < n)
	{
		c = *(str + i);
		bytes += ft_putchar(c);
		i++;
	}
	return (bytes);
}
