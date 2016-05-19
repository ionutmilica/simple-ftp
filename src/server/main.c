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
} conn_handler;

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
 		context_handle(ctx, cmd);
 		command_destroy(cmd);
 	}

 	context_destroy(ctx);
	free(h);

	return NULL;
}

/**
 * Creates a socket and binds
 *
 * @param port int
 * @return int
 */
void server(int port, user_manager* mgr)
{
	int sock = create_socket(port);
	int connection, addr_len;
	struct sockaddr_in client_addr;

	addr_len = sizeof(struct sockaddr_in);

	while ((connection = accept(sock, (struct sockaddr *)&client_addr, (socklen_t*)&addr_len)) ) {
        pthread_t t;
        conn_handler* h = malloc(sizeof(conn_handler));
        h->mgr = mgr;
        h->socket = connection;

        // Spawn a thread and send the connection fd
        printf("Received a new connection!\n");

        if (pthread_create(&t, NULL, handler, (void*) h) < 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
	}
}


int main() 
{
	user_manager* mgr = user_manager_new("users.txt");

	// Start normal server
	server(5555, mgr);

	// Start admin server

	// Start soap server

	user_manager_destroy(mgr);

	return 0;
}