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
t_vec3			parse_xyz(char *str);
t_color			vec3_to_rgb(t_vec3 xyz);
void			put_pixel(t_img *img_data, int x, int y, int color);
void			render(t_img *img_data, int width, int height,
					t_holder *holder);
void			read_scene(int fd, t_holder *window);
unsigned long	rgba_to_hex(t_color rgba);
float			dot(t_vec3 u, t_vec3 v);
t_vec3			normalize(t_vec3 vector);
void			set_color(t_color *color, int r, int g, int b);

#endif
