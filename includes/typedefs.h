/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   typedefs.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <ejuliao-@42lisboa.com>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 08:38:41 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/13 08:38:47 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TYPEDEFS_H
# define TYPEDEFS_H

typedef struct s_vec3
{
	float	x;
	float	y;
	float	z;
}				t_xyz;

typedef struct s_transform
{
	t_xyz	position;
	t_xyz	rotation;
	t_xyz	scale;
}				t_transform;

typedef struct s_color
{
	int	r;
	int	g;
	int	b;
	int	a;
}				t_color;

typedef struct s_sphere
{
	t_transform	transform;
	int			diameter;
	t_color		color;
}				t_sphere;

typedef struct s_plane
{
	t_transform	transform;
	t_color		color;
}				t_plane;

typedef struct s_square
{
	t_transform	transform;
	int			side_size;
	t_color		color;
}				t_square;

typedef struct s_cylinder
{
	t_transform	transform;
	int			diameter;
	int			height;
	t_color		color;
}				t_cylinder;

typedef struct s_triangle
{
	t_xyz	p1;
	t_xyz	p2;
	t_xyz	p3;
	t_color	color;
}				t_triangle;

typedef struct s_objects
{
	void		*object;
	char		*type;
	void		*next;
}				t_objects;

typedef struct s_camera
{
	t_transform	transform;
	int			fov;
}				t_camera;

typedef struct s_light
{
	t_transform	transform;
	int			brightness;
	t_color		color;
}				t_light;

typedef struct s_lights
{
	t_light		light;
	t_light		*next;
}				t_lights;

typedef struct s_scene
{
	int			x_res;
	int			y_res;
	t_light		ambient_clr;
	t_camera	camera;
	t_lights	lights;
	t_sphere	sphere;
	t_objects	objects;
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
	t_xyz	origin;
	t_xyz	direction;
}				t_ray;

#endif
