#include "Scene.hpp"

void	init_Luz(Scene &scene)
{
	scene.setXResolution(1920);
	scene.setYResolution(1080);

	scene.addCamera(Camera(Transform(Vector3(0.0f, 0.0f, 6.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f)), 70));
	scene.addSphere(Sphere());

	// void *mlx_ptr = mlx_init();
	// scene.setMlx(mlx_ptr);

	// MLXImage image;
	// int bpp;
	// int ll;
	// int end;
	// image.setImg(mlx_new_image(scene.getMlx(), scene.getXResolution(), scene.getYResolution()));
	// image.setAddress(mlx_get_data_addr(image.getImg(), &bpp, &ll, &end));
	// image.setBitsPerPixel(bpp);
	// image.setEndian(end);
	// image.setLineLength(ll);

	// scene->window = mlx_new_window(scene->mlx, scene->x_res, scene->y_res, WINDOW_TITLE);
	// mlx_hook(scene->window, DESTROYNOTIFY, 0L, clean_exit, scene);
	// mlx_key_hook(scene->window, window_key_callback, scene);
	// mlx_loop_hook(scene->mlx, render_manager, scene);
	// printf(COLOR_YELLOW "Starting rendering thread%c...\n" COLOR_NC, thread_s);
	// print_render_message(scene);
	// mlx_loop(scene->mlx);
}
