/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minirt.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:16:25 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/11 15:27:54 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINIRT_H
# define MINIRT_H

// Includes
# include <fcntl.h>
# include <sys/stat.h>
# include <stdio.h>
# include <stdbool.h>
# include <float.h>
# include <unistd.h>
# include <math.h>
# include <stdlib.h>
# include <pthread.h>
# include "libft.h"
# include "mlx.h"
# include "get_next_line.h"
# include "typedefs.h"

// Macros
# define WINDOW_TITLE "ejuliao-'s miniRT"

# define COLOR_NC "\e[0m"
# define COLOR_BLACK "\e[0;30m"
# define COLOR_GRAY "\e[1;30m"
# define COLOR_RED "\e[0;31m"
# define COLOR_LIGHT_RED "\e[1;31m"
# define COLOR_GREEN "\e[0;32m"
# define COLOR_LIGHT_GREEN "\e[1;32m"
# define COLOR_BROWN "\e[0;33m"
# define COLOR_YELLOW "\e[1;33m"
# define COLOR_BLUE "\e[0;34m"
# define COLOR_LIGHT_BLUE "\e[1;34m"
# define COLOR_PURPLE "\e[0;35m"
# define COLOR_LIGHT_PURPLE "\e[1;35m"
# define COLOR_CYAN "\e[0;36m"
# define COLOR_LIGHT_CYAN "\e[1;36m"
# define COLOR_LIGHT_GRAY "\e[0;37m"
# define COLOR_WHITE "\e[1;37m"

# ifndef OS
#  define OS 1
# endif

// MacOS = 1
// Linux = 2

# if OS == 1
#  define KEY_ESC 53
#  define KEY_J 106
#  define DESTROYNOTIFY 17
# elif OS == 2
#  define KEY_ESC 65307
#  define KEY_J 106
#  define DESTROYNOTIFY 33
# endif

// Utils
t_color		vec3_to_rgb(t_vec3 xyz);
int			rgba_to_hex(t_color rgba);
t_color		hex_to_rgba(int hex);
void		put_pixel(t_img *img_data, int x, int y, int color);
t_color		get_pixel(t_img *img_data, int x, int y);
float		dot(t_vec3 u, t_vec3 v);
t_vec3		cross(t_vec3 vec1, t_vec3 vec2);
t_vec3		normalize(t_vec3 vector);
bool		is_vec3_in_range(t_vec3 v, float min, float max);
t_color		set_color(int r, int g, int b);
t_color		sum_colors(t_color clr1, t_color clr2);
t_color		mul_color(t_color clr, float m);
t_color		divide_color(t_color clr, float f);
bool		is_color_valid(t_color clr);
t_vec3		sum(t_vec3 vec1, t_vec3 vec2);
t_vec3		sub(t_vec3 vec1, t_vec3 vec2);
t_vec3		mul(t_vec3 vec, float m);
t_vec3		set(float x, float y, float z);
float		length_sqrt(t_vec3 v);
float		length(t_vec3 v);
int			exit_error(t_scene *scene, char *message);
int			clean_exit(t_scene *scene, int code);
int			write_bmp(t_scene *scene);
bool		save_bmp(bool save);
char		*bmp_name(char *file);
char		*get_file_no_ext(const char *path);

// Scene reading
t_vec3		parse_xyz(char *str);
void		read_scene(int fd, t_scene *scene);
void		store_object(t_scene *scene, t_object *object);
void		store_light(t_scene *scene, t_object *object);
void		store_camera(t_scene *scene, t_object *object);
void		verify_values(t_scene *scene, char **input, int start);

// Rendering
int			render_manager(t_scene *scene);
void		*render(void *vscene);
t_vec3		random_in_unit(void);
t_ray		gen_ray(t_scene *scene, t_vec2 pxl, t_vec3 origin, t_vec3 dir);
bool		check_ray_hits(t_scene *scene, t_ray ray, t_hit_record *hit_rec);
void		light_bouncer(t_scene *scene, t_vec2 pxl, t_hit_record *hit_rec);
bool		get_hit_color(t_scene *scene, t_hit_record *hit_rec, int x, int y);
void		calc_lights(t_scene *scene, t_hit_record *hit_rec);

// Object utils
bool		hit_sphere(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
				float t_max);
t_sphere	*get_sphere(t_scene *scene);
bool		hit_plane(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
				float t_max);
bool		intersect_plane(t_plane *plane, t_ray *ray, t_hit_record *hit_rec,
				float t_max, float t_min);
t_plane		*get_plane(t_scene *scene);
bool		hit_cylinder(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
				float t_max);
t_cylinder	*get_cylinder(t_scene *scene);
bool		hit_triangle(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
				float t_max);
t_triangle	*get_triangle(t_scene *scene);
bool		hit_square(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
				float t_max);
t_square	*get_square(t_scene *scene);
t_camera	*get_camera(t_scene *scene);

#endif
