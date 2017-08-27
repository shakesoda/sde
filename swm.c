#include <stk.h>
#include <assert.h>
#include <bcm_host.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "dri.h"
#include "key.h"

#define NAME "/tmp/swm_lock"

static int g_sock = -1;
static int g_donezo = 0;
static struct fb_info *g_infos = NULL;

static void
host_init()
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
host_shutdown()
{
	close(g_sock);
	unlink(NAME);
}

static void
dri_init()
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
dri_shutdown()
{
	// dri cleanup
	swm_dri_shutdown();
	free(g_infos);
}

static int
swm_init()
{
	host_init();

	// sanity check: did we fail to get a usable socket?
	if (g_sock < 0)
	{
		puts("unable to host service, already running?");
		return -1;
	}

	dri_init();
	if (g_infos == NULL)
	{
		return -1;
	}

	return 0;
}

static int
swm_running()
{
	return g_donezo? 0 : 1;
}

static void
swm_shutdown(struct stk_event_t* event)
{
	keyboard_reset();

	puts("thing is kill");
	host_shutdown();
	dri_shutdown();
}

static void
swm_input(struct stk_event_t *event)
{
	if (event->key.code == 27)
	{
		g_donezo = 1;
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
swm_event_pump(struct stk_event_t *event)
{
	memset(event, 0, sizeof(struct stk_event_t));

	int k;
	if (key_pressed(&k))
	{
		event->type = STK_KEYDOWN;
		event->key.code = k;

		return 1;
	}

	return 0;
}

void
swm_push(struct stk_event_t *event)
{
//	for each client
//	{
//		send(event)
//	}
}

int
main(int argc, char **argv)
{
	int err = swm_init();
	if (err != 0)
	{
		return err;
	}

	struct stk_event_t event;
	while (swm_running())
	{
		while (swm_event_pump(&event))
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
			if (event.type != 0)
			{
				swm_push(&event);
			}
		}

		draw();

		// TODO: figure out how to wait for vblank
		usleep(1000);
	}

	// TODO: disconnect clients
	keyboard_reset();

	return 0;
}
