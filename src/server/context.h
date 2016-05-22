#ifndef CONTEXT_H
#define CONTEXT_H

#include "command.h"
#include "user.h"

typedef enum modes {
	NORMAL,
	CLIENT,
	SERVER
} modes;

typedef struct context {
	char user[100];
	int logged_in;
	int permission;
	int fd;
	int pasv_fd;
	int mode;
	user_manager* mgr;
} context;

context* context_new();
void context_destroy(context* ctx);
void context_handle(context* ctx, command* cmd, int type);
void message_send(int fd, const char* message);

#endif