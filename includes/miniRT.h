/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:16:25 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/10 15:28:38 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINIRT_H
# define MINIRT_H

// Includes
# include <fcntl.h>
# include <stdio.h>
# include <stdbool.h>
# include <float.h>
# include <math.h>
# include "libft.h"
# include "mlx.h"
# include "get_next_line.h"
# include "typedefs.h"

// Macros
# define WINDOW_TITLE "ejuliao-'s miniRT"
# define KEY_ESC 53

// Function prototypes
t_xyz			parse_xyz(char *str);
t_color			xyz_to_rgb(t_xyz xyz);
int				exit_error(char *message);
void			put_pixel(t_img *img_data, int x, int y, int color);
void			render(t_img *img_data, int width, int height,
					t_holder *holder);
void			read_scene(int fd, t_holder *window);
unsigned long	rgba_to_hex(t_color rgba);
float			dot(t_xyz u, t_xyz v);

#endif
