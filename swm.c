#include <stk.h>
#include <assert.h>
#include <bcm_host.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "dri.h"

#define NAME "/tmp/swm_lock"

static int g_sock = -1;
static struct fb_info *g_infos = NULL;

static void
init_host()
{
	// force unlink the lock, in case previous run crashed or something...
	// TODO: exit if lock file exists, that's the point.
	unlink(NAME);

	g_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (g_sock < 0)
	{
		puts("oh no");
		return;
	}

	struct sockaddr_un server;
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, NAME);
	if (bind(g_sock, (struct sockaddr*)&server, sizeof(struct sockaddr_un))) {
		puts("ded");
		return;
	}

	listen(g_sock, 5);

	puts("it worked");
}

static void
shutdown_host()
{
	close(g_sock);
	unlink(NAME);
}

static void
init_dri()
{
	g_infos = swm_dri_init();
	if (g_infos[0].base == NULL)
	{
		free(g_infos);
		g_infos = NULL;
		puts("can't into kms");
		return;
	}
}

static void
swm_shutdown(struct stk_event_t* event)
{
	puts("thing is kill");
	shutdown_host();

	// dri cleanup
	swm_dri_shutdown();
	free(g_infos);
}

static void
swm_input(struct stk_event_t *event)
{
	if (event->key.code == 27)
	{
		stk_terminate(0);
	}
}


static void
draw()
{
	int x,y;
	struct fb_info *current = g_infos;
	while (current->base != NULL)
	{
		int col=(rand()%0x00ffffff)&0x00ff00ff;
		int w = current->w;
		int h = current->h;
		for (y=0;y<h;y++)
		{
			for (x=0;x<w;x++)
			{
				int location=y*(w) + x;
				*(((uint32_t*)current->base)+location) = col;
			}
		}
		current++;
	}
}

int
main(int argc, char **argv)
{
	stk_init();

	init_host();

	// sanity check: did we fail to get a usable socket?
	if (g_sock < 0)
	{
		puts("unable to host service, already running?");
		return -1;
	}

	init_dri();
	if (g_infos == NULL)
	{
		return -1;
	}

	struct stk_event_t event;
	while (stk_running())
	{
		while (stk_event_pump(&event))
		{
			switch (event.type)
			{
			case STK_KEYDOWN:
				swm_input(&event);
				break;
			case STK_SHUTDOWN:
				swm_shutdown(NULL);
				break;
			default:
				break;
			}
		}

		draw();
		usleep(1000);
	}

	return stk_run();
}
