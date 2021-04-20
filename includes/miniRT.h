/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:16:25 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/20 11:12:54 by ejuliao-         ###   ########.fr       */
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

# ifndef OS
#  define OS 1
# endif

// MacOS = 1
// Linux = 2

# if OS == 1
#  define KEY_ESC 53
#  define OS_BRIGHTNESS_FACTOR 1.0f
# elif OS == 2
#  define KEY_ESC 65307
#  define OS_BRIGHTNESS_FACTOR 0.3f
# endif

// Function prototypes
t_vec3	parse_xyz(char *str);
t_color	vec3_to_rgb(t_vec3 xyz);
void	put_pixel(t_img *img_data, int x, int y, int color);
void	render(t_img *img_data, int width, int height, t_holder *holder);
void	read_scene(int fd, t_holder *window);
int		rgba_to_hex(t_color rgba);
float	dot(t_vec3 u, t_vec3 v);
t_vec3	normalize(t_vec3 vector);
void	set_color(t_color *color, int r, int g, int b);
int		exit_error(char *message);
int		clean_exit(t_holder *holder);

#endif
