/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putstr_bonus.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/08 09:56:38 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/08 17:38:54 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_putstr(const char *str)
{
	int		bytes;
	int		i;
	char	c;

	i = 0;
	bytes = 0;
	while (*(str + i) != '\0')
	{
		c = *(str + i);
		bytes += ft_putchar(c);
		i++;
	}
	return (bytes);
}
