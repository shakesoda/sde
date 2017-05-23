#pragma once

enum stk_event_type_t
{
	STK_NONE,
	STK_KEYDOWN,
	STK_SHUTDOWN,
	STK_INVALID
};
typedef enum stk_event_type_t stk_event_type_t;

// basic generic SDL-style event
struct stk_event_t
{
	stk_event_type_t type;
	union
	{
		struct
		{
			int code;
		} key;
	};
};
typedef struct stk_event_t stk_event_t;

//typedef void (*stk_callback_t)(stk_event_t*);
int stk_running(void);

// prepare stk for use (allocate callback memory, etc)
void stk_init(void);

// terminate with return code
void stk_terminate(int);

// run main loop, or finish execution and return if stk_terminate() has been called.
int stk_run(void);

// check for program events
int stk_event_pump(stk_event_t *event);
