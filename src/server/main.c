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
#include "context.h"
#include "command.h"

/**
 * Creates a socket and binds
 *
 * @param port int
 * @return int
 */
int create_socket(int port)
{
	struct sockaddr_in server_addr;
	int sock, reuse = 1;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Cannot open socket\n");
		exit(EXIT_FAILURE);
	}

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);

	if (bind(sock,(struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		perror("Cannot bind socket to address");
		exit(EXIT_FAILURE);
	}

  	listen(sock, 5);

  	return sock;
}

void* handler(void* data) {
	int sock = *(int*) data, bytes_read;
	char buffer[BUFFER_SIZE];

	// Send welcome message	
	char* welcome = "220 Hello boss!\n";
 	write(sock, welcome, strlen(welcome));

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
	server(5555);

	return 0;
}