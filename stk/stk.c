#include <stk.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h> // usleep
#include <string.h> // mem*
#include <stdlib.h>
#include "key.h"

static int retcode = 0;
static bool donezo = false;
static bool queued_shutdown = false;

void
stk_terminate(int code)
{
	donezo = true;
	queued_shutdown = true;
	retcode = code;
}

static void
sigh(int signo)
{
	switch (signo)
	{
	case SIGINT:
		stk_terminate(-1);
		break;
	default: break;
	}
}

int
stk_running()
{
	return donezo? 0 : 1;
}

int
stk_event_pump(struct stk_event_t *event)
{
	memset(event, 0, sizeof(struct stk_event_t));

	int k;
	if (key_pressed(&k))
	{
		event->type = STK_KEYDOWN;
		event->key.code = k;

		return 1;
	}

	if (donezo && queued_shutdown)
	{
		event->type = STK_SHUTDOWN;
		queued_shutdown = false;

		return 1;
	}

	return 0;
}

void
stk_init()
{
	struct sigaction sa;
	sa.sa_handler = sigh;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
}

int
stk_run()
{
	struct stk_event_t event;

	while (!donezo) {
		// poll events
		stk_event_pump(&event);
		if (donezo) {
			break;
		}
		// run input
		// update layout
		// draw
		// wait for next event
		usleep(1000);
	}

	keyboard_reset();

	return retcode;
}

struct stk_window_t*
stk_window_create(unsigned flags)
{
	struct stk_window_t *wnd = calloc(1, sizeof(struct stk_window_t));

	// TODO: notify WM process

	return wnd;
}

void
stk_window_destroy(struct stk_window_t *wnd)
{
	// TODO: notify WM process

	free(wnd);
}
