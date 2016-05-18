#include "command.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** 
 * Creates a new command
 * @return command*
 */
command* command_new() {
	command* cmd = malloc(sizeof(command));
	return cmd;
}

/** 
 * Parse the command from a given string
 * @return void
 */
void command_parse(command* cmd, const char* str) {
	sscanf(str,"%s %s", cmd->command, cmd->arg);
	// Remove the new line if it exist
	size_t len = strlen(cmd->arg);
	if (cmd->arg[len - 1] == '\n') {
		cmd->arg[len - 1] = '\0';
	}
}

/** 
 * Free the command
 * @return void
 */
void command_destroy(command* cmd) {
	if (cmd != NULL) free(cmd);
}

int command_find(const char* name) {
	int i, n = sizeof(commands) / sizeof(char *);
	for (i = 0; i < n; i++) {
		if (!strcmp(name, commands[i])) {
			return i;
		}
	}
	return -1;
}
