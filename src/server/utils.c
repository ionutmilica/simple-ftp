#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
		perror("Cannot open socket\n");
		exit(EXIT_FAILURE);
	}

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);

	if (bind(sock,(struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
		printf("On port %d:", port);
		perror("Cannot bind socket to address");
		exit(EXIT_FAILURE);
	}

  	listen(sock, 5);

  	return sock;
}
