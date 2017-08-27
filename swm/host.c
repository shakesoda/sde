#include <stk.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "host.h"

#define NAME "/tmp/swm_lock"

static int g_sock = -1;
static struct swm_client *g_clients = NULL;
static int g_max_clients = 1;

struct swm_client*
get_clients()
{
	return g_clients;
}

int
host_init()
{
	// force unlink the lock, in case previous run crashed or something...
	// TODO: exit if lock file exists, that's the point.
	unlink(NAME);

	g_sock = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (g_sock < 0)
	{
		puts("oh no");
		return 1;
	}

	struct sockaddr_un server;
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, NAME);
	if (bind(g_sock, (struct sockaddr*)&server, sizeof(struct sockaddr_un))) {
		puts("ded");
		return 1;
	}

	listen(g_sock, 5);

	g_clients = calloc(sizeof(struct swm_client), g_max_clients);
	memset(g_clients, 0, sizeof(struct swm_client) * g_max_clients);
	for (int i = 0; i < g_max_clients; i++)
	{
		g_clients[i].sock = -1;
	}

	puts("it worked");

	return 0;
}

void
host_shutdown()
{
	for (int i = 0; i < g_max_clients; i++)
	{
		if (g_clients[i].sock >= 0)
		{
			// TODO: push disconnect/shutdown message first
			close(g_clients[i].sock);
		}
	}

	free(g_clients);
	g_clients = NULL;

	close(g_sock);
	g_sock = -1;

	unlink(NAME);
}

static void
try_accept(fd_set *rfds)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int st;
	for (;;)
	{
		st = select(1, rfds, NULL, NULL, &tv);
		if (!st || st == -1)
		{
			break;
		}

		// add/remove from g_clients
		struct sockaddr_un client;
		size_t s = sizeof(client);
		int sock = -1;
		if ((sock = accept(g_sock, (struct sockaddr*)&client, &s)) == -1)
		{
			puts("error accepting connections");
			break;
		}

		if (g_clients[g_max_clients].sock < 0)
		{
			g_max_clients *= 2;
			g_clients = realloc(g_clients, sizeof(struct swm_client)*g_max_clients);
		}
	}
}

static void
client_read(fd_set *rfds, struct swm_client *client)
{
	STK_UNUSED(client);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int st;
	for (;;)
	{
		st = select(1, rfds, NULL, NULL, &tv);
		if (!st || st == -1)
		{
			break;
		}
	}
}

void
host_service()
{

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(g_sock, &rfds);
	
	// accept new connections
	try_accept(&rfds);

	// handle messages from clients
	// TODO: set up the fd_set properly instead of being stupid
	for (int i = 0; i < g_max_clients; i++)
	{
		struct swm_client *client = &g_clients[i];
		if (client->sock < 0)
			continue;

		FD_ZERO(&rfds);
		FD_SET(client->sock, &rfds);

		client_read(&rfds, client);
	}
}
