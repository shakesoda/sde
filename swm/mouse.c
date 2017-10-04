#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "mouse.h"
//#include <linux/input.h>

static int g_fd = -1;

void
mouse_init()
{
	g_fd = open("/dev/input/mice", O_RDONLY | O_NONBLOCK);
}

void
mouse_poll()
{
	uint8_t data[3];
	while (read(g_fd, data, sizeof(data)) > 0)
	{
		bool left = (data[0] & 0x1) > 0,
			right = (data[0] & 0x2) > 0,
			middle = (data[0] & 0x4) > 0;
		bool x1 = (data[0] & (1 << 3)) > 0;
		bool x2 = (data[0] & (1 << 4)) > 0;
		bool x3 = (data[0] & (1 << 5)) > 0;
		bool x4 = (data[0] & (1 << 6)) > 0;
		bool x5 = (data[0] & (1 << 7)) > 0;
		int8_t x = data[1], y = data[2];
		printf("n: %d %d %d %d %d  ", x, y, left, right, middle);
		printf("x: %d %d %d %d %d\n", x1, x2, x3, x4, x5);
	}
}

void
mouse_shutdown()
{
	close(g_fd);
}
