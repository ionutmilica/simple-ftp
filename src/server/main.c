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

void* handler(void* data) {
	int sock = *(int*) data, bytes_read;
	char buffer[BUFFER_SIZE];

	// Send welcome message	
 	message_send(sock, "220 Welcome!\n");

 	context* ctx = context_new();
 	ctx->fd = sock;

 	// read the commands from the client
 	while ((bytes_read = read(sock, buffer, BUFFER_SIZE))) {
 		buffer[BUFFER_SIZE - 1] = '\0';

 		command* cmd = command_new();
 		command_parse(cmd, buffer);
 		context_handle(ctx, cmd);
 		command_destroy(cmd);
 	}

 	context_destroy(ctx);
	free(data);

	return NULL;
}

/**
 * Creates a socket and binds
 *
 * @param port int
 * @return int
 */
void server(int port)
{
	int sock = create_socket(port);
	int connection, addr_len;
	struct sockaddr_in client_addr;

	addr_len = sizeof(struct sockaddr_in);

	while ((connection = accept(sock, (struct sockaddr *)&client_addr, (socklen_t*)&addr_len)) ) {
        pthread_t t;
		int* connection_fd = (int*) malloc(sizeof(int));
        *connection_fd = connection;

        // Spawn a thread and send the connection fd
        printf("Received a new connection!\n");

        if (pthread_create(&t, NULL, handler, (void*) connection_fd) < 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
	}
}


int main() 
{
	user_manager* mgr = user_manager_new("users.txt");

	// Start normal server
	server(5555);

	// Start admin server

	// Start soap server

	user_manager_destroy(mgr);

	return 0;
}