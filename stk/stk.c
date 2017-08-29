
#include <stk.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // usleep
#include <string.h> // mem*
#include <sys/msg.h>
#include <sys/ipc.h>

// from <linux/msg.h>
#define MSG_COPY 040000 /* copy (not remove) all queue messages */
#define KEY_NAME "/tmp/swm_lock"

static int g_pid = -1;
static int g_sendq = -1;
static int g_recvq = -1;

static int retcode = 0;
static bool donezo = false;
static bool queued_shutdown = false;

static struct stk_msg
message_new(enum stk_wm_msg_t type, int wid)
{
	struct stk_msg msg = { 0 };
	msg.type = type;
	msg.pid = g_pid;
	msg.wid = wid;
	return msg;
}

static void
message_send(struct stk_msg *buf)
{
	if (msgsnd(g_sendq, buf, sizeof(struct stk_msg), IPC_NOWAIT) == -1)
	{
		puts("Unable to connect to display server.");
		exit(1); // should this be non-fatal?
		return;
	}
}

// pass -1 to ignore window id
static int
message_find(struct stk_msg *buf, int wid)
{
	// with multiple messages in the queue for other processes this will be awful...
	if (msgrcv(g_recvq, buf, sizeof(struct stk_msg), 0, IPC_NOWAIT) >= 0)
	{
		if (buf->pid == g_pid && (wid < 0 || buf->wid == wid))
		{
			return 1;
		}
		// no match, put it back
		msgsnd(g_recvq, buf, sizeof(struct stk_msg), IPC_NOWAIT);
	}

	return 0;
}

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

	struct stk_msg buf = message_new(STK_WM_INVALID, 0);
	if (message_find(&buf, -1))
	{
		// closed by WM
		if (buf.type == STK_WM_PROC_DESPAWN)
		{
			puts("this would be a good time to close");
			stk_terminate(0);
			return 1;
		}
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
stk_init(int argc, char **argv)
{
	STK_UNUSED(argc);
	STK_UNUSED(argv);

	g_pid = getpid();

	struct sigaction sa;
	sa.sa_handler = sigh;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	key_t send_key = ftok(KEY_NAME, 's');
	key_t recv_key = ftok(KEY_NAME, 'r');

	g_sendq = msgget(send_key, 0644);
	g_recvq = msgget(recv_key, 0644);

	struct stk_msg buf = message_new(STK_WM_PROC_SPAWN, 0);
	message_send(&buf);
}

static void
disconnect()
{
	struct stk_msg buf = message_new(STK_WM_PROC_DESPAWN, 0);
	message_send(&buf);
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

static int g_wid = 0;

struct stk_window_t*
stk_window_create(unsigned flags)
{
	struct stk_window_t *wnd = calloc(1, sizeof(struct stk_window_t));
	wnd->flags = flags;
	wnd->id = g_wid++;

	struct stk_msg buf = message_new(STK_WM_OPEN, wnd->id);
	message_send(&buf);

	return wnd;
}

void
stk_window_destroy(struct stk_window_t *wnd)
{
	struct stk_msg buf = message_new(STK_WM_CLOSE, wnd->id);
	message_send(&buf);

	free(wnd);
}
