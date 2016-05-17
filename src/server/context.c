#include "context.h"
#include "message.h"
#include <stdlib.h>
#include <stdio.h>

context* context_new() {
	context* ctx = malloc(sizeof(context));
	ctx->logged_in = 0;
	ctx->fd = 0;

	return ctx;
}

void context_destroy(context* ctx) {
	if (ctx != NULL) {
		free(ctx);
	}
}

void context_handle(context* ctx, command* cmd) {
	message* msg;
	printf("Command: %s, Arg: %s\n", cmd->command, cmd->arg);

	switch (command_find(cmd->command)) {
		case AUTH:
			message_send(ctx->fd, "502 Not implemented\n");
		break;
		case USER:
			// Check username then respond
			message_send(ctx->fd, "331 User name okay, need password.\n");
		break;
		case PASS:
			ctx->logged_in = 1;
			message_send(ctx->fd, "230 Login successful\n");
		break;
		default:
			message_send(ctx->fd, "502 Not implemented\n");
	}
}