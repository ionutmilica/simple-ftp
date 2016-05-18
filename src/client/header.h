#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_LEN 512
#define BSIZE 1024

typedef struct Response
{
  int code;
  char message[1024];
} Response;

typedef struct Command
{
  char command[5];
  char arg[1024];
} Command;


void clean_string(char*);
void recv_response(int, Response*);
void parse_response(char*, Response*);
void parse_command(char*, Command*);
void send_file(int, char*);
int execute_command(Command *);

int ftp_pasv(int);
int ftp_login(int, char*, char*);
int ftp_send_file(int, char*);
int ftp_recv_file(int, char*);
int ftp_remove_file(int, char*);
int ftp_ls_firectory(int, char*);