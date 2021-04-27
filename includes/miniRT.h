/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   miniRT.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/08 15:16:25 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/27 11:21:04 by ejuliao-         ###   ########.fr       */
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
# define RENDERING_MSG "Rendering..."

# ifndef OS
#  define OS 1
# endif

// MacOS = 1
// Linux = 2

# if OS == 1
#  define KEY_ESC 53
#  define DESTROYNOTIFY 17
# elif OS == 2
#  define KEY_ESC 65307
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
t_vec3		unit_vector(t_vec3 vector);
void		set_color(t_color *color, int r, int g, int b);
t_vec3		sum(t_vec3 vec1, t_vec3 vec2);
t_vec3		sub(t_vec3 vec1, t_vec3 vec2);
t_vec3		set(float x, float y, float z);
int			exit_error(char *message);
int			clean_exit(t_scene *scene);
int			write_bmp(t_scene *scene);
bool		save_bmp(bool save);
char		*bmp_name(char *file);
char		*get_file_no_ext(const char *path);

// Scene reading
t_vec3		parse_xyz(char *str);
void		read_scene(int fd, t_scene *scene);
void		store_object(t_scene *scene, t_object *object);

// Rendering
int			manage_frames(t_scene *scene);
void		*render(void *vscene);
t_vec3		random_in_unit(void);
t_ray		gen_ray(t_scene *scene, t_vec3 uv, t_vec3 origin, t_vec3 dir);
bool		check_ray_hits(t_scene *scene, t_ray ray, t_color *hit_color,
				t_hit_record *hit_rec);
void		light_bouncer(t_scene *scene, t_vec3 uv, t_color *hit_color,
				t_hit_record *hit_rec);
bool		get_hit_color(t_scene *scene, t_hit_record *hit_rec,
				t_color *hit_color, t_vec3 crnt_pxl);

// Object utils
bool		hit_sphere(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
				float t_max);
t_sphere	*get_sphere(t_scene *scene);
bool		hit_plane(t_scene *scene, t_ray *ray, t_hit_record *hit_rec,
				float t_max);
t_plane		*get_plane(t_scene *scene);
t_gnrc_obj	*get_gnrc_obj(t_scene *scene);

#endif
