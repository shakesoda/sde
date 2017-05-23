#include <stk.h>
#include <assert.h>
#include <bcm_host.h>
#include <stdbool.h>
#include <stdio.h>

static DISPMANX_DISPLAY_HANDLE_T display;

void
shutdown(stk_event_t* event)
{
	puts("thing is kill");

	int result = vc_dispmanx_display_close(display);
	assert(result == 0);
}

void
input(stk_event_t *event)
{
	if (event->key.code == 27)
	{
		stk_terminate(0);
	}
}

int
main(int argc, char **argv)
{
	stk_init();
	bcm_host_init();

	display = vc_dispmanx_display_open(0);
	assert(display != 0);

	int result;
	DISPMANX_MODEINFO_T info;
	result = vc_dispmanx_display_get_info(display, &info);
	assert(result == 0);

	DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
	assert(update != 0);

	result = vc_dispmanx_update_submit_sync(update);
	assert(result == 0);

	stk_event_t event;
	while (stk_running())
	{
		while (stk_event_pump(&event))
		{
			switch (event.type)
			{
			case STK_KEYDOWN:
				input(&event);
				break;
			case STK_SHUTDOWN:
				shutdown(NULL);
				break;
			default:
				break;
			}
		}

		// update frame
		update = vc_dispmanx_update_start(0);
		assert(update != 0);

		result = vc_dispmanx_update_submit_sync(update);
		assert(result == 0);
	}

	return stk_run();
}
