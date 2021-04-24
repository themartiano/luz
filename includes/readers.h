/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readers.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 13:14:30 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/24 19:59:41 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READERS_H
# define READERS_H

bool		read_sp(char **values, t_scene *scene);
bool		read_pl(char **values, t_scene *scene);
bool		read_sq(char **values, t_scene *scene);
bool		read_cy(char **values, t_scene *scene);
bool		read_tr(char **values, t_scene *scene);

#endif
