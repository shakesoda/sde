#pragma once
#include <stdbool.h>

// a non-blocking function that returns true if a character has been read
// from stdin. If the character argument is not NULL, the character read is
// returned. NOTE when function keys, arrow keys etc are pressed the function
// will return true, but the character argument will not be set.
bool key_pressed(int *character);

// put the stdin stream back to the way it was found when the program started
void keyboard_reset(void);
