
#include <stk.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // usleep
#include <string.h> // mem*
//#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#define KEY_NAME "/tmp/swm_lock"

static int g_sendq = -1;
static int g_recvq = -1;

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

	if (donezo && queued_shutdown)
	{
		event->type = STK_SHUTDOWN;
		queued_shutdown = false;

		return 1;
	}

	return 0;
}

void
stk_init(int argc, char **argv)
{
	STK_UNUSED(argc);
	STK_UNUSED(argv);

	struct sigaction sa;
	sa.sa_handler = sigh;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	key_t send_key = ftok(KEY_NAME, 's');
	key_t recv_key = ftok(KEY_NAME, 'r');

	g_sendq = msgget(send_key, 0644);
	g_recvq = msgget(recv_key, 0644);

	struct stk_msg buf;
	buf.type = 1; // hello

	if (msgsnd(g_sendq, &buf, sizeof(struct stk_msg), IPC_NOWAIT) == -1)
	{
		puts("unable to connect to server");
		exit(1);
		return;
	}

	printf("connected\n");
}

static void
disconnect()
{
	struct stk_msg buf;
	buf.type = 2; // goodbye
	msgsnd(g_sendq, &buf, sizeof(struct stk_msg), IPC_NOWAIT);
	// send dc to queue...
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

	disconnect();

	return retcode;
}

struct stk_window_t*
stk_window_create(unsigned flags)
{
	struct stk_window_t *wnd = calloc(1, sizeof(struct stk_window_t));

	STK_UNUSED(flags);
	// TODO: notify WM process

	return wnd;
}

void
stk_window_destroy(struct stk_window_t *wnd)
{
	// TODO: notify WM process

	free(wnd);
}
