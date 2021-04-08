/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/08 15:44:04 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/08 17:40:52 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strtrim(const char *s1, const char *set)
{
	char	*trim_str;
	size_t	i;
	size_t	j;
	size_t	k;

	if (s1 == NULL || set == NULL)
		return (NULL);
	i = 0;
	while (s1[i] && ft_charinset(s1[i], set))
		i++;
	j = ft_strlen(s1);
	while (j > i && ft_charinset(s1[j - 1], set))
		j--;
	trim_str = malloc(((j - i) + 1) * sizeof(char));
	if (trim_str == NULL)
		return (NULL);
	k = 0;
	while (i < j)
		trim_str[k++] = s1[i++];
	trim_str[k] = '\0';
	return (trim_str);
}
