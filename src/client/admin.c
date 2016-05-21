#include <sys/un.h>
#include "header.h"

#define SV_SOCK_PATH "/tmp/ftp"


int main(int argc, char *argv[]){
  struct sockaddr_un addr;
  int sfd, cfd;
  ssize_t numRead;
  char buf[BSIZE];
  char command[256];
  int code;

  if((sfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
    printf("Open socket error!\n");
    exit(0);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path)-1);

  if(connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) != 0){
    perror("Connect ");
    exit(0);
  }

  while(1){
      Command *cmd = malloc(sizeof(Command));
      fflush(stdin);
      fgets(command, sizeof(command), stdin);
      clean_string(command);

      parse_command(command, cmd);
      
      code = execute_command(cmd, sfd);
  }
}  