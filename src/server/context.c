#include "common.h"
#include "context.h"
#include "builtin.h"

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h>


context* context_new() {
	context* ctx = malloc(sizeof(context));
	ctx->logged_in = 0;
	ctx->fd = -1;
	ctx->pasv_fd = -1;
	ctx->mode = NORMAL;
	ctx->permission = 0;

	return ctx;
}

void context_destroy(context* ctx) {
	if (ctx != NULL) {
		if (ctx->pasv_fd != -1) {
			close(ctx->pasv_fd);
		}
		free(ctx);
	}
}

void message_send(int fd, const char* message) {
	write(fd, message, strlen(message));
}

void context_handle(context* ctx, command* cmd, int type) {
	printf("Command: %s, Arg: %s\n", cmd->command, cmd->arg);

	if (type != 1) {
		ctx->permission = 0;
	}

	switch (command_find(cmd->command)) {
		case AUTH:
			message_send(ctx->fd, "502 Not implemented\n");
		break;
		case USER:
			// Check username then respond
			cmd_user(ctx, cmd);
			break;
		case PASS:
			cmd_pass(ctx, cmd);
			break;
		case PWD:
			cmd_pwd(ctx, cmd);
			break;
		case CWD:
			cmd_cwd(ctx, cmd);
			break;
		case TYPE:
			cmd_type(ctx, cmd); 
			break;
		case PORT:
			message_send(ctx->fd, "502 Not implemented\n");
			break;
		case PASV:
			cmd_pasv(ctx, cmd);
			break;
		case LIST:
			cmd_list(ctx, cmd);
			break;
		case RETR:
			cmd_retr(ctx, cmd);
			break;
		case STOR:
			cmd_stor(ctx, cmd);
			break;
		case DELE:
			cmd_dele(ctx, cmd);
			break;
		case USERS:
			cmd_users(ctx, cmd);
			break;
		default:
			message_send(ctx->fd, "502 Not implemented\n");
	}
}