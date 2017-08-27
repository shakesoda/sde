#pragma once

#define STK_UNUSED(x) (void)x

enum stk_event_type_t
{
	STK_NONE,
	STK_KEYDOWN,
	STK_SHUTDOWN,
	STK_INVALID
};

enum stk_window_flags_t
{
	STK_WF_NONE,
	STK_WF_NORMAL = 1 << 0,
	// STK_WF_FULLSCREEN = 1 << 1,
	// STK_WF_ALWAYS_ON_TOP = 1 << 2,
	STK_WF_INVALID = 1 << 3
};

// basic generic SDL-style event
struct stk_event_t
{
	enum stk_event_type_t type;
	union
	{
		struct
		{
			int code;
		} key;
	};
};

struct stk_window_t
{
	unsigned width;
	unsigned height;
	unsigned flags;
};

// prepare stk for use (allocate callback memory, etc)
void stk_init(int argc, char **argv);

// terminate with return code
void stk_terminate(int);

// run main loop, or finish execution and return if stk_terminate() has been called.
int stk_run(void);

// returns 1 until terminate() has been called for any reason
int stk_running(void);

// check for program events
int stk_event_pump(struct stk_event_t *event);

struct stk_window_t *stk_window_create(unsigned flags);

void stk_window_destroy(struct stk_window_t *wnd);
