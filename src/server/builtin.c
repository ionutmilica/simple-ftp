#define _GNU_SOURCE
#include "builtin.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/sendfile.h>

#include "user.h"

void cmd_pasv(context* ctx, command* cmd) 
{
	int port, p1, p2;
	int ip[4] = {127, 0, 0, 1};
	char buffer[BUFFER_SIZE];

	if (!ctx->logged_in) {
		message_send(ctx->fd, "530 Please login with USER and PASS.\n");
		return;
	}

	// @todo: Generate the port
	port = 5666;
	p1 = ((port >> 8) & 0xFF);
	p2 = (port & 0xFF);

	if (ctx->pasv_fd != -1) {
		close(ctx->pasv_fd);
	}

	sprintf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\n", ip[0], ip[1], ip[2], ip[3], p1, p2);
	ctx->mode = SERVER;
	ctx->pasv_fd = create_socket(port);

	message_send(ctx->fd, buffer);
}

void cmd_list(context* ctx, command* cmd) 
{
	struct dirent *entry;
	struct stat statbuf;
	struct tm *time;
	char timebuff[80];
	int connection;
	time_t rawtime;

	char cwd[BUFFER_SIZE], cwd_orig[BUFFER_SIZE];

	if (!ctx->logged_in) {
		message_send(ctx->fd, "530 Please login with USER and PASS.\n");
		return;
	}

	memset(cwd, 0, BUFFER_SIZE);
	memset(cwd_orig, 0, BUFFER_SIZE);

	getcwd(cwd_orig, BUFFER_SIZE);

	if (strlen(cmd->arg) > 0 && cmd->arg[0] != '-') {
		chdir(cmd->arg);
	}

	getcwd(cwd, BUFFER_SIZE);
	DIR *dp = opendir(cwd);

	if (!dp) {
	  message_send(ctx->fd, "550 Failed to open directory.\n");
	}

	if (ctx->mode == SERVER) {
		struct sockaddr_in client_address;
		int addrlen = sizeof(client_address);

		message_send(ctx->fd, "150 Sending the data.\n");

		connection = accept(ctx->pasv_fd, (struct sockaddr*) &client_address, (socklen_t*)&addrlen);

		while ((entry = readdir(dp))) {
			if (stat(entry->d_name, &statbuf) == -1) {
				fprintf(stderr, "FTP: Error reading file stats...\n");
			} else {
				char perms[9];
				memset(perms, 0, 9);

				rawtime = statbuf.st_mtime;
				time = localtime(&rawtime);
				strftime(timebuff,80,"%b %d %H:%M",time);
			 	perm((statbuf.st_mode & ALLPERMS), perms);

				dprintf(connection,
					"%c%s %5d %4d %4d %8d %s %s\r\n", 
					(entry->d_type==DT_DIR)?'d':'-',
					perms,
					(int) statbuf.st_nlink,
					(int) statbuf.st_uid, 
					(int) statbuf.st_gid,
					(int) statbuf.st_size,
					timebuff,
					entry->d_name);
			}
		}
		close(connection);
		close(ctx->pasv_fd);
		ctx->mode = NORMAL;
		message_send(ctx->fd, "226 Directory send OK.\n");
	} else if (ctx->mode == CLIENT) {
		message_send(ctx->fd, "502 Command not implemented.\n");
	} else {
		message_send(ctx->fd, "425 Use PASV or PORT first.\n");
	}

	closedir(dp);
	chdir(cwd_orig);
}

void cmd_users(context* ctx, command* cmd) {
	int connection;

	if (!ctx->logged_in) {
		close(ctx->pasv_fd);
		message_send(ctx->fd, "530 Please login with USER and PASS.\n");
		return;
	}

	if (!ctx->permission != 1) {
		close(ctx->pasv_fd);
		message_send(ctx->fd, "530 You are not admin.\n");
		return;
	}

	if (ctx->mode == SERVER) {
		struct sockaddr_in client_address;
		int addrlen = sizeof(client_address);

		message_send(ctx->fd, "150 Sending the data.\n");

		connection = accept(ctx->pasv_fd, (struct sockaddr*) &client_address, (socklen_t*)&addrlen);

		int i;
		for (i = 0; i < ctx->mgr->length; i++) {
			dprintf(connection, "%s %d\r\n", ctx->mgr->users[i].name, ctx->mgr->users[i].perm);
		}

		close(connection);
		close(ctx->pasv_fd);
		ctx->mode = NORMAL;
		message_send(ctx->fd, "226 Directory send OK.\n");
	} else if (ctx->mode == CLIENT) {
		message_send(ctx->fd, "502 Command not implemented.\n");
	} else {
		message_send(ctx->fd, "425 Use PASV or PORT first.\n");
	}
}

void cmd_retr(context* ctx, command* cmd) 
{
	int fd, conn;
	struct stat stat_buf;
	off_t offset = 0;
	int sent_total = 0;

	if (!ctx->logged_in) {
		message_send(ctx->fd, "530 Please login with USER and PASS.\n");
		return;
	}

	if (ctx->mode == SERVER) {
		if(access(cmd->arg,R_OK)==0 && (fd = open(cmd->arg,O_RDONLY))){
			struct sockaddr_in client_address;
			int addrlen = sizeof(client_address);

			fstat(fd, &stat_buf);
			message_send(ctx->fd, "150 Opening BINARY mode data connection.\n");

			conn = accept(ctx->pasv_fd, (struct sockaddr*) &client_address, (socklen_t*)&addrlen);
			close(ctx->pasv_fd);

			if ((sent_total = sendfile(conn, fd, &offset, stat_buf.st_size))) {
				message_send(ctx->fd, "226 File send OK.\n");
			} else {
				message_send(ctx->fd, "550 Failed to read file.\n");
			}
			close(fd);
			close(conn);
		} else {
			message_send(ctx->fd, "550 Failed to get file\n");
		}
	} else {
		message_send(ctx->fd, "550 Please use PASV instead of PORT.\n");
	}

	ctx->mode = NORMAL;
	close(ctx->pasv_fd);
}

void cmd_stor(context* ctx, command* cmd) 
{
	int conn, fd, pipefd[2], res = 1;
	FILE *file;
	const int buff_size = 8192;
	
	if (!ctx->logged_in) {
		message_send(ctx->fd, "530 Please login with USER and PASS.\n");
		return;
	}

	file = fopen(cmd->arg, "w");

	if (file == NULL) {
		message_send(ctx->fd, "451 Requested action aborted. Local error in processing.\n");
	} else if (ctx->mode != SERVER) {
		message_send(ctx->fd, "550 Please use PASV instead of PORT.\n");
	} else {
		struct sockaddr_in client_address;
		int addrlen = sizeof(client_address);

		fd = fileno(file);
		conn = accept(ctx->pasv_fd, (struct sockaddr*) &client_address, (socklen_t*)&addrlen);

		// After we accept the client we should be sure that we don't accept other connections into this port
		close(ctx->pasv_fd);

		if (pipe(pipefd) == -1) {
			perror("ftp_stor: pipe");
		}

		message_send(ctx->fd, "125 Data connection already open; transfer starting.\n");

		while ((res = splice(conn, 0, pipefd[1], NULL, buff_size, SPLICE_F_MORE | SPLICE_F_MOVE)) > 0) {
			splice(pipefd[0], NULL, fd, 0, buff_size, SPLICE_F_MORE | SPLICE_F_MOVE);
		}

		if (res == -1) {
			message_send(ctx->fd, "451 Requested action aborted. Local error in processing.\n");
		} else {
			message_send(ctx->fd, "226 File send OK.\n");
		}

		close(conn);
		close(fd);
	}

	ctx->mode = NORMAL;
	close(ctx->pasv_fd);
}

void cmd_dele(context* ctx, command* cmd) 
{
	if (!ctx->logged_in) {
		message_send(ctx->fd, "530 Please login with USER and PASS.\n");
		return;
	}

	if (unlink(cmd->arg) == -1) {
		message_send(ctx->fd, "550 File unavailable.\n");
	} else {
		message_send(ctx->fd, "250 Requested file action okay, completed.\n");
	}
}

void cmd_type(context* ctx, command* cmd) 
{
	if (!ctx->logged_in) {
		message_send(ctx->fd, "530 Please login with USER and PASS.\n");
		return;
	}
	if (cmd->arg[0] == 'I') {
		message_send(ctx->fd, "200 Switching to Binary mode.\n");
	} else if (cmd->arg[0] == 'A') {
		message_send(ctx->fd, "200 Switching to ASCII mode.\n");
	} else {
		message_send(ctx->fd, "504 Command not implemented for that parameter.\n");
	}
}

void cmd_cwd(context* ctx, command* cmd) 
{
	if (!ctx->logged_in) {
		message_send(ctx->fd, "530 Please login with USER and PASS.\n");
		return;
	}
	if (!chdir(cmd->arg)) {
		message_send(ctx->fd, "250 Directory successfully changed.\n");
	} else {
		message_send(ctx->fd, "550 Failed to change directory.\n");
	}
}

void cmd_pwd(context* ctx, command* cmd) 
{
	char cwd[BUFFER_SIZE], result[BUFFER_SIZE];
	memset(result, 0, BUFFER_SIZE);

	if (!ctx->logged_in) {
		message_send(ctx->fd, "530 Please login with USER and PASS.\n");
		return;
	}

	if (getcwd(cwd, BUFFER_SIZE) != NULL) {
		strcat(result, "257 \"");
		strcat(result, cwd);
		strcat(result, "\"\n");
		message_send(ctx->fd, result);
	} else {
		message_send(ctx->fd, "550 Failed to get pwd.\n");
	}
}

void cmd_user(context* ctx, command* cmd) 
{
	if (!user_manager_find(ctx->mgr, cmd->arg)) {
		message_send(ctx->fd, "530 Invalid username\n");
		return;
	}
	// Save the username for future uses
	strcpy(ctx->user, cmd->arg);
	message_send(ctx->fd, "331 User name okay, need password.\n");
}

void cmd_pass(context* ctx, command* cmd) 
{
	if (!user_manager_find_with_pass(ctx->mgr, ctx->user, cmd->arg)) {
		message_send(ctx->fd, "500 Invalid username or password\n");
		return;
	}

	user u;
	user_manager_get(ctx->mgr, ctx->user, &u);

	ctx->logged_in = 1;
	ctx->permission = u.perm;

	if (u.perm == 1) {
		message_send(ctx->fd, "230 Login successful. You are admin!\n");
	} else {
		message_send(ctx->fd, "230 Login successful\n");
	}
}
