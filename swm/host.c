#include <stk.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "host.h"

#define KEY_NAME "/tmp/swm_lock"

static int g_sendq = -1;
static int g_recvq = -1;

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
	while ((rd = msgrcv(g_recvq, &buf, sizeof(struct stk_msg), 0, IPC_NOWAIT)) >= 0)
	{
		printf("type %d\n", buf.type);
	}
}
