#include <stk.h>

int
main(int argc, char **argv)
{
	STK_UNUSED(argc);
	STK_UNUSED(argv);

	stk_init();

	return stk_run();
}
