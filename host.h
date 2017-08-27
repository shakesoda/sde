#pragma once

struct swm_client
{
	int sock;
};

struct swm_client* get_clients();
// returns nonzero on failure
int host_init();
void host_shutdown();
void host_service();
