#ifndef CONTEXT_H
#define CONTEXT_H

#include "command.h"

typedef enum modes {
	NORMAL,
	CLIENT,
	SERVER
} modes;

typedef struct context {
	int logged_in;
	int fd;
	int pasv_fd;
	int mode;
} context;

context* context_new();
void context_destroy(context* ctx);
void context_handle(context* ctx, command* cmd);
void message_send(int fd, const char* message);

#endif