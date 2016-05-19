#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

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

void perm(int perm, char* str_perm)
{
	int curperm = 0, i, read, write, exec;
	char fbuff[3];

	read = write = exec = 0;

	for (i = 6; i >= 0; i -= 3) {
		curperm = ((perm & ALLPERMS) >> i ) & 0x7;
		memset(fbuff,0,3);
		
		read = (curperm >> 2) & 0x1;
		write = (curperm >> 1) & 0x1;
		exec = (curperm >> 0) & 0x1;

		sprintf(fbuff, "%c%c%c", read ? 'r' : '-', write ? 'w' : '-', exec ? 'x' : '-');
		strcat(str_perm,fbuff);
	}
}