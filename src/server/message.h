#ifndef MESSAGE_H
#define MESSAGE_H

#include "common.h"

typedef struct message {
	int code;
	char message[BUFFER_SIZE];
} message;

message* message_new();
message* message_from(int code, const char* text);
void message_write(int fd, message* msg);
void message_destroy(message* msg);

#endif