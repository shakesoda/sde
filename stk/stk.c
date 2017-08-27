#include <stk.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h> // usleep
#include <string.h> // mem*
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

static int retcode = 0;
static bool donezo = false;
static bool queued_shutdown = false;
static int g_sock = -1;

#define SOCK_PATH "/tmp/swm_lock"

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

	int s, len;
	struct sockaddr_un remote;

	if ((s = socket(AF_UNIX, SOCK_SEQPACKET, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	printf("trying to connect...\n");

	struct stat buf;
	s = stat(SOCK_PATH, &buf);
	if (s != 0)
	{
		fprintf(stderr, "SWM doesn't appear to be running.\n");
		exit(1);
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr *)&remote, len) == -1) {
		perror("connect");
		exit(1);
	}

	g_sock = s;

	printf("connected\n");
}

static void
disconnect()
{
	close(g_sock);
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
