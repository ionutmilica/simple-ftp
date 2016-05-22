#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <dirent.h>
#include <time.h>

#include "common.h"
#include "utils.h"
#include "context.h"
#include "command.h"
#include "user.h"

typedef struct conn_handler {
	user_manager* mgr;
	int socket;
	int type;
} conn_handler;

typedef struct server_info {
	user_manager* mgr;
	int port;
	char sock_path[BUFFER_SIZE];
} server_info;

void* handler(void* data) {
	conn_handler* h = (conn_handler*) data;
	int bytes_read;
	char buffer[BUFFER_SIZE];

	// Send welcome message	
 	message_send(h->socket, "220 Welcome!\n");

 	context* ctx = context_new();
 	ctx->fd = h->socket;
 	ctx->mgr = h->mgr;

 	// read the commands from the client
 	while ((bytes_read = read(h->socket, buffer, BUFFER_SIZE))) {
 		buffer[BUFFER_SIZE - 1] = '\0';

 		command* cmd = command_new();
 		command_parse(cmd, buffer);
 		context_handle(ctx, cmd, h->type);
 		command_destroy(cmd);
 	}

 	context_destroy(ctx);
	free(h);

	return NULL;
}

/**
 * Creates the server
 *
 * @param port int
 * @return void
 */
void* server(void* data)
{
	server_info info = *(server_info*) data;
	user_manager* mgr = info.mgr;

	int port, sock;
	int connection, addr_len;
	struct sockaddr_in client_addr;

	sock = create_socket(info.port);
	addr_len = sizeof(struct sockaddr_in);

	while ((connection = accept(sock, (struct sockaddr *)&client_addr, (socklen_t*)&addr_len)) ) {
		pthread_t t;
		conn_handler* h = malloc(sizeof(conn_handler));
		h->mgr = mgr;
		h->socket = connection;
		h->type = 0;

		// Spawn a thread and send the connection fd
		printf("Received a new connection in the normal server!\n");

		if (pthread_create(&t, NULL, handler, (void*) h) < 0) {
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}
	}

	free(data);

	return NULL;
}

void* admin_server(void* data)
{
	server_info info = *(server_info*) data;
	user_manager* mgr = info.mgr;
	int sock, connection, addr_len;
	struct sockaddr_in client_addr;

	unlink(info.sock_path);

	sock = create_named_socket(info.sock_path);
	addr_len = sizeof(struct sockaddr_in);

	while ((connection = accept(sock, (struct sockaddr *)&client_addr, (socklen_t*)&addr_len)) ) {
		pthread_t t;
		conn_handler* h = malloc(sizeof(conn_handler));
		h->mgr = mgr;
		h->socket = connection;
		h->type = 1;

		// Spawn a thread and send the connection fd
		printf("Received a new connection in the admin server!\n");

		if (pthread_create(&t, NULL, handler, (void*) h) < 0) {
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}
	}

	free(data);

	return NULL;
}

int main() 
{
	pthread_t servers[3];
	server_info* info;
	user_manager* mgr = user_manager_new("users.txt");


	/** Start socket server **/
	info = malloc(sizeof(server_info));
	info->port = 5555;
	info->mgr = mgr;
	pthread_create(&servers[0], NULL, server, (void*) info);

	/** Start named socket server **/
	info = malloc(sizeof(server_info));
	strcpy(info->sock_path, "/tmp/ftp");
	info->mgr = mgr;
	pthread_create(&servers[1], NULL, admin_server, (void*) info);


	// Wait for the servers to finish (in theory never)
	pthread_join(servers[0], NULL);
	pthread_join(servers[0], NULL);
	
	user_manager_destroy(mgr);

	return 0;
}