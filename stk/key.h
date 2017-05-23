#pragma once
#include <stdbool.h>

// The keyPressed function is a non-blocking function that returns true if
// a character has been read from stdin. If the character argument is not
// NULL, the character read is returned. NOTE when function keys, arrow keys
// etc are pressed the function will return true, but the character argument
// will not be set.
bool keyPressed(int *character);

// The keyboardReset function puts the stdin stream back to the way it was
// found when the program started.
void keyboardReset(void);
