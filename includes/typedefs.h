/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   typedefs.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/13 08:38:41 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 09:12:10 by ejuliao-         ###   ########.fr       */
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
	t_color			color;
	t_transform		transform;
}				t_plane;

typedef struct s_sphere
{
	t_color			color;
	t_transform		transform;
	int				radius;
}				t_sphere;

typedef struct s_square
{
	t_color			color;
	t_transform		transform;
	int				side_size;
}				t_square;

typedef struct s_cylinder
{
	t_color				color;
	t_transform			transform;
	int					radius;
	int					height;
}				t_cylinder;

typedef struct s_triangle
{
	t_color				color;
	t_vec3				p1;
	t_vec3				p2;
	t_vec3				p3;
}				t_triangle;

typedef struct s_camera
{
	t_transform	transform;
	int			fov;
	float		half_width;
	float		half_height;
}				t_camera;

typedef struct s_light
{
	t_color			color;
	t_transform		transform;
	float			brightness;
}				t_light;

/* ******************** *\
	| Object types:  |
	|----------------|
	| Sphere:		0|
	| Plane:		1|
	| Square:		2|
	| Cylinder:		3|
	| Triangle:		4|
	| Light:		5|
	|----------------|
\* ******************** */

typedef struct s_object
{
	void	*object;
	int		type;
	void	*prev;
	void	*next;
}				t_object;

typedef struct s_img
{
	void	*img;
	char	*addr;
	int		bits_per_pixel;
	int		line_length;
	int		endian;
}				t_img;

typedef struct s_scene
{
	void		*mlx;
	void		*window;
	t_img		img;
	pthread_t	thread;
	int			x_res;
	int			y_res;
	float		t_min;
	float		t_max;
	int			samples;
	int			max_bounces;
	t_light		amb_light;
	t_camera	camera;
	t_object	*objects;
	t_object	*lights;
}				t_scene;

typedef struct s_ray
{
	t_vec3	origin;
	t_vec3	direction;
}				t_ray;

typedef struct s_hit_record
{
	float	t;
	t_ray	ray;
	t_vec3	p;
	t_vec3	normal;
	bool	hit;
	t_color	color;
}				t_hit_record;

#endif
