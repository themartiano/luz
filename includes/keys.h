/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keys.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/13 12:13:55 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 06:24:58 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef KEYS_H
# define KEYS_H

# if OS == 1
#  define DESTROYNOTIFY 17

#  define KEY_ESC 53
#  define KEY_J 38
#  define KEY_W 13
#  define KEY_A 0
#  define KEY_S 1
#  define KEY_D 2
#  define KEY_UP 126
#  define KEY_LEFT 123
#  define KEY_DOWN 125
#  define KEY_RIGHT 124
#  define KEY_Q 12
#  define KEY_E 14
# elif OS == 2
#  define DESTROYNOTIFY 33

#  define KEY_ESC 65307
#  define KEY_J 106
#  define KEY_W 119
#  define KEY_A 97
#  define KEY_S 115
#  define KEY_D 100
#  define KEY_UP 65362
#  define KEY_LEFT 65361
#  define KEY_DOWN 65364
#  define KEY_RIGHT 65363
#  define KEY_Q 113
#  define KEY_E 101
# endif

#endif
