#include <stk.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "host.h"

#define KEY_NAME "/tmp/swm_lock"

static int g_sendq = -1;
static int g_recvq = -1;

static struct stk_msg
message_new(enum stk_wm_msg_t type, int pid, int wid)
{
	struct stk_msg msg = { 0 };
	msg.type = type;
	msg.pid = pid;
	msg.wid = wid;
	return msg;
}

int
host_init()
{
	// NB: these are reversed relative to STK clients, as we're the server.
	key_t send_key = ftok(KEY_NAME, 'r');
	key_t recv_key = ftok(KEY_NAME, 's');

	g_sendq = msgget(send_key, 0644 | IPC_CREAT);
	g_recvq = msgget(recv_key, 0644 | IPC_CREAT);

	return 0;
}

static void
message_send(struct stk_msg *buf)
{
	if (msgsnd(g_sendq, buf, sizeof(struct stk_msg)-sizeof(long), IPC_NOWAIT) == -1)
	{
		puts("ow");
		return;
	}
}

void
host_shutdown()
{
	msgctl(g_sendq, IPC_RMID, 0);
	msgctl(g_recvq, IPC_RMID, 0);
}

void
host_service()
{
	struct stk_msg buf = { 0 };
	int rd;
	while ((rd = msgrcv(g_recvq, &buf, sizeof(struct stk_msg)-sizeof(long), 0, IPC_NOWAIT)) >= 0)
	{
		printf("message from %ld (%d):\n", buf.pid, buf.wid);
		printf("\ttype %d\n", buf.type);

		if (buf.type != STK_WM_PROC_DESPAWN)
		{
			struct stk_msg kill = message_new(STK_WM_PROC_DESPAWN, buf.pid, 0);
			message_send(&kill);
		}
	}
}
