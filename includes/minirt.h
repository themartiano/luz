/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minirt.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:16:25 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/05/14 14:52:54 by ejuliao-         ###   ########.fr       */
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
# include "colors.h"

// Macros
# define WINDOW_TITLE "ejuliao-'s miniRT"
# define CAMERA_MOVE_STEP 1.0f
# define CAMERA_ROTATE_STEP 0.02f

# ifndef OS
#  define OS 1 // MacOS = 1, Linux = 2
# endif

# include "keys.h"

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
void		print_render_message(t_scene *scene);
void		change_camera(t_scene *scene);
void		move_camera(t_scene *scene, int keycode);
void		rotate_camera(t_scene *scene, int keycode);
t_vec3		divide(t_vec3 vec, float d);

// Scene reading
t_vec3		parse_xyz(char *str);
void		read_scene(int fd, t_scene *scene);
void		store_object(t_scene *scene, t_object *object);
void		store_light(t_scene *scene, t_object *object);
void		store_camera(t_scene *scene, t_object *object);
void		verify_values(t_scene *scene, char **input, int start);
void		check_resolution(t_scene *scene);
void		check_for_integer(t_scene *scene, char *str);

// Rendering
int			render_manager(t_scene *scene);
void		*render(void *vscene);
t_vec3		random_in_unit(void);
t_ray		gen_ray(t_scene *scene, t_vec2 pxl, t_vec3 origin, t_vec3 dir);
bool		check_ray_hits(t_scene *scene, t_ray ray, t_hit_record *hit_rec);
void		light_bouncer(t_scene *scene, t_vec2 pxl, t_hit_record *hit_rec);
bool		get_hit_color(t_scene *scene, t_hit_record *hit_rec, int x, int y);
void		calc_lights(t_scene *scene, t_hit_record *hit_rec);
void		set_hit_color(t_scene *scene, t_hit_record *hit_rec);
void		clear_rendering_thread(t_scene *scene);

// Object utils
bool		hit_sphere(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
				float t_max);
t_sphere	*get_sphere(t_scene *scene);
bool		hit_plane(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
				float t_max);
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
