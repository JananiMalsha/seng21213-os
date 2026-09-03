#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "../../include/types.h"

void kb_init(void);
char kb_getchar(void);
int  kb_readline(char *buf, int len);

#endif
