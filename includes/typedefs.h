/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   typedefs.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 08:38:41 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/19 14:46:11 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TYPEDEFS_H
# define TYPEDEFS_H

typedef struct s_vec3
{
	float	x;
	float	y;
	float	z;
}				t_vec3;

typedef struct s_transform
{
	t_vec3	position;
	t_vec3	orientation;
	t_vec3	scale;
}				t_transform;

typedef struct s_color
{
	int	r;
	int	g;
	int	b;
	int	a;
}				t_color;

typedef struct s_plane
{
	t_transform		transform;
	t_color			color;
	struct s_plane	*prev;
	struct s_plane	*next;
}				t_plane;

typedef struct s_sphere
{
	t_transform		transform;
	int				radius;
	t_color			color;
	struct s_sphere	*prev;
	struct s_sphere	*next;
}				t_sphere;

typedef struct s_square
{
	t_transform		transform;
	int				side_size;
	t_color			color;
	struct s_square	*prev;
	struct s_square	*next;
}				t_square;

typedef struct s_cylinder
{
	t_transform			transform;
	int					radius;
	int					height;
	t_color				color;
	struct s_cylinder	*prev;
	struct s_cylinder	*next;
}				t_cylinder;

typedef struct s_triangle
{
	t_vec3				p1;
	t_vec3				p2;
	t_vec3				p3;
	t_color				color;
	struct s_triangle	*prev;
	struct s_triangle	*next;
}				t_triangle;

typedef struct s_camera
{
	t_transform	transform;
	int			fov;
}				t_camera;

typedef struct s_light
{
	t_transform		transform;
	int				brightness;
	t_color			color;
	struct s_light	*next;
}				t_light;

typedef struct s_scene
{
	int			x_res;
	int			y_res;
	t_light		ambient_clr;
	t_camera	camera;
	t_light		light;
	t_plane		*plane;
	t_sphere	*sphere;
	t_square	*square;
	t_cylinder	*cylinder;
	t_triangle	*triangle;
}				t_scene;

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
	t_scene	scene;
}				t_holder;

typedef struct s_ray
{
	t_vec3	origin;
	t_vec3	direction;
}				t_ray;

#endif
