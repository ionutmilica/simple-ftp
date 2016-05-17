#include "message.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

message* message_new() {
	message* msg = malloc(sizeof(message));
	msg->code = -1;
	memset(msg->message, '\0', sizeof(msg->message));
	return msg;
}

message* message_from(int code, const char* text) {
	message* msg = message_new();
	msg->code = code;
	strcpy(msg->message, text);

	return msg;
}

void message_write(int fd, message* msg) {
	char buffer[BUFFER_SIZE];
	snprintf(buffer, sizeof(buffer), "%d %s\n", msg->code, msg->message);
	printf("Message: %s\n", buffer);
	write(fd, buffer, strlen(buffer));
}

void message_send(int fd, const char* message) {
	write(fd, message, strlen(message));
}

void message_destroy(message* msg) {
	if (msg != NULL) {
		free(msg);
	}
}