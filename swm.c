#include <stk.h>
#include <assert.h>
#include <bcm_host.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "dri.h"

#define NAME "/tmp/swm_lock"

static DISPMANX_DISPLAY_HANDLE_T display;
static DISPMANX_UPDATE_HANDLE_T update;

static void
swm_shutdown(struct stk_event_t* event)
{
	puts("thing is kill");

	int result = vc_dispmanx_display_close(display);
	assert(result == 0);
}

static void
swm_input(struct stk_event_t *event)
{
	if (event->key.code == 27)
	{
		stk_terminate(0);
	}
}

void
init_dispmanx()
{
	int result;
	DISPMANX_MODEINFO_T info;
	result = vc_dispmanx_display_get_info(display, &info);
	assert(result == 0);

	update = vc_dispmanx_update_start(0);
	assert(update != 0);

	result = vc_dispmanx_update_submit_sync(update);
	assert(result == 0);
}

void
init_kms()
{
	puts("use kms");
}

int
main(int argc, char **argv)
{
	stk_init();
	bcm_host_init();

	unlink(NAME);

	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
	{
		puts("oh no");
		goto skip;
	}

	struct sockaddr_un server;
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, NAME);
	if (bind(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_un))) {
		puts("ded");
		goto skip;
	}

	listen(sock, 5);

	puts("it worked");

	close(sock);
	unlink(NAME);

skip:

	display = vc_dispmanx_display_open(0);

	// If this fails, we're most likely using a KMS driver, not BCM.
	bool use_dispmanx = true;
	if (display != 0)
	{
		init_dispmanx();
	}
	else
	{
		init_kms();
		use_dispmanx = false;
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

		// update frame
		if (use_dispmanx)
		{
			update = vc_dispmanx_update_start(0);
			assert(update != 0);

			int result = vc_dispmanx_update_submit_sync(update);
			assert(result == 0);
		}
	}

	return stk_run();
}

int
xmain(int argc, char *argv[])
{
	struct fb_info *infos = setup_dri();
	if (infos[0].base == NULL)
	{
		puts("oh nooooo");
		return -1;
	}

	int x,y;
	for (int i=0;i<100;i++)
	{
		struct fb_info *current = infos;
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
		usleep(100000);
	}

	free(infos);

	return 0;
}
