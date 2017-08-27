#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

static int g_fd = -1;

void
mouse_init()
{
	g_fd = open("/dev/input/mice", O_RDONLY);
}

void
mouse_poll()
{

}

void
mouse_shutdown()
{
	close(g_fd);
}
