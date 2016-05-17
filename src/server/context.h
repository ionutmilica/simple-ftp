#ifndef CONTEXT_H
#define CONTEXT_H

#include "command.h"

typedef struct context {
	int logged_in;
	int fd;
} context;

context* context_new();
void context_destroy(context* ctx);
void context_handle(context* ctx, command* cmd);

#endif