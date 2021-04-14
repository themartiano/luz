/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readers.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 13:14:30 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/13 13:14:31 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef READERS_H
# define READERS_H

bool		read_pl(char **values, t_holder *holder);
bool		read_sq(char **values, t_holder *holder);
bool		read_cy(char **values, t_holder *holder);
bool		read_tr(char **values, t_holder *holder);

#endif
