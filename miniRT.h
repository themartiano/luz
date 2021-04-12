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
# include "libft.h"
# include "mlx.h"
# include "get_next_line.h"

// Macros
# define WINDOW_TITLE "ejuliao-'s miniRT"

// Structs
typedef struct s_img
{
	void	*img;
	char	*addr;
	int		bits_per_pixel;
	int		line_length;
	int		endian;
}				t_img;

typedef struct s_holder
{
	void	*mlx;
	void	*window;
	t_img	img;
	int		width;
	int 	height;
}				t_holder;

// Function prototypes
int		exit_error(char *message);
void	put_pixel(t_img *img_data, int x, int y, int color);
void	fill_image(t_img *img_data, int width, int height, int color);
void	read_scene(int fd, t_holder *window);

#endif
