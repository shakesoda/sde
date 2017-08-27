#pragma once

struct fb_info
{
	void *base;
	long w;
	long h;
};

// returns zero-initialized list of fb_infos, check .base for null and remember to free()
struct fb_info* swm_dri_init();
void swm_dri_shutdown();
