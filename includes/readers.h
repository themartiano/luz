/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readers.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 13:14:30 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 10:25:54 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READERS_H
# define READERS_H

bool		read_sp(char **values, char *line, t_scene *scene);
bool		read_pl(char **values, char *line, t_scene *scene);
bool		read_sq(char **values, char *line, t_scene *scene);
bool		read_cy(char **values, char *line, t_scene *scene);
bool		read_tr(char **values, char *line, t_scene *scene);

#endif
