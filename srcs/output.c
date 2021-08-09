#include "minirt.h"

static double	getexectime(void);

void	*output_manager(void *vscene)
{
	t_scene	*scene = (t_scene *)vscene;

	getexectime();
	int		grid_size = sqrt(scene->thread_count - 1);
	float	percentage = 0.0f;
	do
	{
		percentage = (float)(scene->rendered_rows / grid_size / (scene->y_res / 100.0f));
		if (percentage > 100.0f) percentage = 100.0f;
		if (percentage < 0.0f) percentage = 0.0f;
		printf(COLOR_WHITE "\r[ %.0f%% ]" COLOR_NC, percentage);
		fflush(stdout);
		usleep(10000);
	} while (percentage < 100.0f);
	printf(COLOR_LIGHT_GREEN "\n\nRender done! " COLOR_LIGHT_BLUE "(Duration: "
		COLOR_WHITE "%.2fs" COLOR_LIGHT_BLUE ")\n\n" COLOR_NC, getexectime());
	write_bmp(scene);
	return (NULL);
}

double	getexectime(void)
{
	struct timeval	time;
	static uint64_t	start_time = 0;
	uint64_t		current_time;

	gettimeofday(&time, NULL);
	current_time = (time.tv_sec * (uint64_t)1000) + (time.tv_usec / 1000);
	if (start_time == 0)
		start_time = current_time;
	return ((current_time - start_time) / 1000.0);
}
