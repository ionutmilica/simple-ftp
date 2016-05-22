#include "header.h"

void parse_response(char *rspstring, Response *rsp)
{
    sscanf(rspstring,"%d %[^\n]",&rsp->code,rsp->message);
}

void parse_command(char *cmdstring, Command *cmd)
{
    sscanf(cmdstring,"%s %[^\n]",cmd->command,cmd->arg);
}

void clean_string(char* string) {
   int l = strlen(string)-1;
   if(string[l]='\n') 
      string[l]='\0';
   else 
      string[l+1]='\0';
}

void recv_response(int sock, Response *response){
    char buffer[1], responsebuff[BSIZE];
    int bytes_read, i = 0;

    memset(buffer, 0, BSIZE);
    while(bytes_read = read(sock, buffer, 1)){
        responsebuff[i] = buffer[0];
        if(buffer[0] == '\n'){
            break;
        }
        i++;
/*        if(bytes_read <= BSIZE){
            parse_response(buffer, response);
        }*/
    }
    parse_response(responsebuff, response);
	printf("%s", responsebuff);
	memset(responsebuff, 0, BSIZE);
}

void send_file(int sock, char* filename){
    int fd;
    int sent_bytes = 0;
    char file_size[256];
    struct stat file_stat;
    int offset;
    int remain_data;
    ssize_t len;

    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
            fprintf(stderr, "Error opening file --> %s", strerror(errno));
            exit(EXIT_FAILURE);
    }

    /* Get file stats */
    if (fstat(fd, &file_stat) < 0)
    {
            fprintf(stderr, "Error fstat --> %s", strerror(errno));
            exit(EXIT_FAILURE);
    }

    offset = 0;
    remain_data = file_stat.st_size;
    /* Sending file data */
    while (((sent_bytes = sendfile(sock, fd, &offset, BUFSIZ)) > 0) && (remain_data > 0))
    {
            remain_data -= sent_bytes;
            fprintf(stdout, "Client sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, offset, remain_data);
    }
    
    close(sock);
}
