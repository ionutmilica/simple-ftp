#ifndef BUILTIN_H
#define BUILTIN_H

#include "context.h"
#include "command.h"

void cmd_pasv(context* ctx, command* cmd);
void cmd_type(context* ctx, command* cmd);
void cmd_cwd(context* ctx, command* cmd);
void cmd_pwd(context* ctx, command* cmd);
void cmd_user(context* ctx, command* cmd);
void cmd_pass(context* ctx, command* cmd);
void cmd_list(context* ctx, command* cmd);
void cmd_retr(context* ctx, command* cmd);
void cmd_stor(context* ctx, command* cmd);

#endif