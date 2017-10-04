#include <stk.h>
#include <stdint.h> // uint*_t
#include <stdio.h> // puts, printf, ...
#include <stdlib.h> // rand, free, *alloc
#include <string.h> // memset
#include <unistd.h> // usleep
#include <signal.h>
#include "host.h"
#include "dri.h"
#include "key.h"
#include "mouse.h"

static int g_no_dri = 0;
static int g_donezo = 0;
static struct fb_info *g_infos = NULL;

static void
sigh(int signo)
{
	switch (signo)
	{
	case SIGKILL:
	case SIGTERM:
	case SIGINT:
		g_donezo = 1;
		break;
	default: break;
	}
}

static int
swm_init()
{
	// sanity check: did we fail to get a usable socket?
	if (host_init() != 0)
	{
		puts("Unable to host service, is it already running?");
		return -1;
	}

	struct sigaction sa;
	sa.sa_handler = sigh;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	mouse_init();

	if (g_no_dri)
	{
		return 0;
	}

	// dri init
	g_infos = swm_dri_init();
	if (g_infos[0].base == NULL)
	{
		free(g_infos);
		puts("Can't initialize DRI/KMS. This is likely a driver or permissions problem.");
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
swm_shutdown()
{
	puts("Shutting down");

	host_shutdown();
	keyboard_reset();
	mouse_shutdown();

	if (!g_no_dri)
	{
		swm_dri_shutdown();
		free(g_infos);
	}
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
	if (g_no_dri)
	{
		return;
	}

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
	host_service();

	memset(event, 0, sizeof(struct stk_event_t));

	int k;
	if (key_pressed(&k))
	{
		event->type = STK_KEYDOWN;
		event->key.code = k;

		return 1;
	}

	mouse_poll();

	return 0;
}

void
swm_push(struct stk_event_t *event)
{
	STK_UNUSED(event);
//	for each client
//	{
//		send(event)
//	}
}

int
main(int argc, char **argv)
{
	for (int i = 0; i < argc; i++)
	{
		const char *arg = argv[i];
		if (strcmp(arg, "--headless") == 0)
		{
			g_no_dri = 1;
			continue;
		}
	}

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
			default:
				break;
			}
			// we could push events next frame, but adding latency is bad.
			if (event.type != 0)
			{
				swm_push(&event);
			}
		}

		draw();

		// TODO: figure out how to wait for vblank
		usleep(1000);
	}

	swm_shutdown();

	return 0;
}
