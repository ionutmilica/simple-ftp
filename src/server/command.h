#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"

typedef struct Command {
	char command[5];
	char arg[BUFFER_SIZE];
} command;

typedef enum commands_enum { 
  AUTH, USER, PASS, PWD, CWD, TYPE, PORT, PASV, LIST, RETR, STOR, DELE, USERS
} commands_enum;

static const char* commands[] = {
	"AUTH", "USER", "PASS", "PWD", "CWD", "TYPE", "PORT", "PASV", "LIST", "RETR", "STOR", "DELE", "USERS",
};

command* command_new();
void command_parse(command* cmd, const char* str);
void command_destroy(command* cmd);
int command_find(const char* name);

#endif