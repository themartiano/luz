/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/23 10:49:39 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/26 16:01:39 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

bool	str_contains_nl(char *str)
{
	size_t	i;

	i = 0;
	if (str == NULL)
		return (false);
	while (str[i] != '\0')
	{
		if (str[i] == '\n')
			return (true);
		i++;
	}
	return (false);
}

size_t	gnl_strlen(const char *s)
{
	size_t	len;

	if (!s)
		return (0);
	len = 0;
	while (s[len])
	{
		len++;
	}
	return (len);
}

char	*gnl_strdup(const char *s)
{
	char	*ptr;
	int		i;

	ptr = (char *)malloc((gnl_strlen(s) + 1) * sizeof(char));
	if (ptr == NULL)
		return (NULL);
	i = 0;
	while (s[i] != '\0')
	{
		ptr[i] = s[i];
		i++;
	}
	ptr[i] = '\0';
	return (ptr);
}

char	*gnl_strjoin(char *s1, const char *s2)
{
	char	*new_str;
	int		i;
	int		j;

	if (s1 == NULL)
		return (gnl_strdup(s2));
	new_str = malloc((gnl_strlen(s1) + gnl_strlen(s2) + 1) * sizeof(char));
	if (new_str == NULL)
		return (NULL);
	i = -1;
	while (s1[++i] != '\0')
		new_str[i] = s1[i];
	j = 0;
	while (s2[j] != '\0')
		new_str[i++] = s2[j++];
	free(s1);
	new_str[i] = '\0';
	return (new_str);
}
