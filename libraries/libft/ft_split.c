/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/08 17:04:32 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 16:00:15 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static char	count_occurrences(const char *str, char c)
{
	int	i;
	int	occurrences;

	i = 0;
	occurrences = 0;
	while (str[i])
	{
		if (str[i] != c && (!str[i + 1] || str[i + 1] == c))
		{
			occurrences++;
		}
		i++;
	}
	return (occurrences);
}

static char	**split(const char *str, char c, int occs, char **split_strs)
{
	int	i;
	int	occ;
	int	chars;

	i = 0;
	occ = 0;
	while (occ < occs)
	{
		while (str[i] && str[i] == c)
			i++;
		chars = 0;
		while (str[i + chars] && str[i + chars] != c)
			chars++;
		split_strs[occ] = malloc((chars + 1) * sizeof(char));
		if (split_strs[occ] == NULL)
			return (NULL);
		chars = 0;
		while (str[i] && str[i] != c)
			split_strs[occ][chars++] = str[i++];
		split_strs[occ++][chars] = '\0';
	}
	split_strs[occ] = 0;
	return (split_strs);
}

char	**ft_split(const char *s, char c)
{
	int		occ;
	char	**split_strs;

	if (s == NULL)
		return (NULL);
	occ = count_occurrences(s, c);
	split_strs = malloc((occ + 1) * sizeof(char *));
	if (split_strs == NULL)
		return (NULL);
	if (split(s, c, occ, &split_strs[0]) == NULL)
		return (NULL);
	return (split_strs);
}
