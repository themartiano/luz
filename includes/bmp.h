/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bmp.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ejuliao- <martinez@brhaka.com>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/26 11:13:09 by ejuliao-          #+#    #+#             */
/*   Updated: 2021/04/29 10:53:51 by ejuliao-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BMP_H
# define BMP_H

# pragma pack(push, 1)

typedef struct s_file_h
{
	short int		type;
	int				size;
	int				reserved;
	unsigned int	offset;
}				t_file_h;

typedef struct s_info_h
{
	unsigned int	size;
	int				width;
	int				height;
	short int		planes;
	short int		bits_per_pxl;
	unsigned int	compression;
	unsigned int	image_size;
	int				pxls_per_meter_x;
	int				pxls_per_meter_y;
	unsigned int	color_used;
	unsigned int	color_important;
}				t_info_h;

# pragma pack(pop)

#endif
