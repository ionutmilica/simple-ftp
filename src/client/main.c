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

int sock_opd;

void clean_string(char*);
void recv_response(int, Response*);
void parse_response(char*, Response*);
void parse_command(char*, Command*);
void send_file(int, char*);
int execute_command(Command *);

int ftp_login(int, char*, char*);
int ftp_send_file(int, char*);
int ftp_recv_file(int, char*);
int ftp_remove_file(int, char*);
int ftp_ls_firectory(int, char*);

int main(int argc, char* argv[])
{   
    struct sockaddr_in sv_adress;
    char command[256];
    char t[1], rdl[1024];
    int code;
    
    
    if(argc<5)
    {
        printf("Usage:%s [server_adress] [port] [username] [password]\n",argv[0]);
        return 1;
    }

    //Open socket
    sock_opd = socket(AF_INET, SOCK_STREAM , 0);
    if(sock_opd < 0){
        printf("Open socket error!\n");
        exit(0);
    }

    //Set server adress/PORT
    sv_adress.sin_family = AF_INET;
    sv_adress.sin_port = htons(atoi(argv[2]));
    if (inet_aton(argv[1], &sv_adress.sin_addr.s_addr) == 0 )
    {
        perror(argv[1]);
        exit(0);
    }

    //Connect to server
    if (connect(sock_opd, (struct sockaddr*)&sv_adress, sizeof(sv_adress)) != 0 )
    {
        perror("Connect ");
        exit(0);
    }

    code = ftp_login(sock_opd, argv[3], argv[4]);
    if(code == 230){
        printf("Client connected.\n");
    }
    
    while(1){
        Command *cmd = malloc(sizeof(Command));
        fgets(command, sizeof(command), stdin);
        clean_string(command);
        
        parse_command(command, cmd);
        
        code = execute_command(cmd);
    }
    //Close socket
    close(sock_opd);

    return 0;
}

int ftp_login(int sock, char* username, char* password){
    char msg[BSIZE];
    char *username_msg = "USER %s";
    char *password_msg = "PASS %s";
    Response *response = malloc(sizeof(Response));

    recv_response(sock, response);
    if(response->code == 220){
        sprintf(msg, username_msg, username);
        send(sock, msg, sizeof(msg), 0);
    }else{
        return response->code;
    }
    memset(msg, 0, BSIZE);

    recv_response(sock, response);
    if(response->code == 331){
        sprintf(msg,password_msg,password);
        send(sock, msg, sizeof(msg), 0);
    }else{
        return response->code;
    }
    memset(msg, 0, BSIZE);
    
    recv_response(sock, response);
    return response->code;
}

int ftp_send_file(int sock, char* filepath){
    int ip[4], p1, p2, socksend, fd;
    struct sockaddr_in sv_adress;
    char buff[BSIZE];
    char ipadr[12];
    char *cmd = "PASV";
    char *cmdfn = "STOR %s";
    Response *response = malloc(sizeof(Response)); 
    
    send(sock, cmd, sizeof(cmd), 0);
    
    recv_response(sock, response);
    if(response->code == 227){
        sscanf(response->message,"%d,%d,%d,%d,%d,%d",&ip[0],&ip[1],&ip[2],&ip[3],&p1,&p2);
        sprintf(ipadr, "%d.%d.%d.%d", ip[0],ip[1],ip[2],ip[3]);
        
        socksend = socket(AF_INET, SOCK_STREAM , 0);
        if(socksend < 0){
            printf("Open socket error!\n");
            exit(0);
        }

        sv_adress.sin_family = AF_INET;
        sv_adress.sin_port = 256 * p1 + p2;
        if (inet_aton(ipadr, &sv_adress.sin_addr.s_addr) == 0 )
        {
            perror(ipadr);
            exit(0);
        }

        if (connect(socksend, (struct sockaddr*)&sv_adress, sizeof(sv_adress)) != 0 )
        {
            perror("Connect ");
            exit(0);
        }
        printf("Client connected to send file.\n");
        
        sprintf(buff, cmdfn, filepath);
        send(sock, buff, sizeof(buff), 0);
        send_file(socksend, filepath);

        recv_response(sock, response);
        return response->code;
        
    }else{
        return response->code;
    }
}

void parse_response(char *rspstring, Response *rsp)
{
    sscanf(rspstring,"%d %s",&rsp->code,rsp->message);
}

void parse_command(char *cmdstring, Command *cmd)
{
  sscanf(cmdstring,"%s %s",cmd->command,cmd->arg);
}

void clean_string(char* string) {
   int l = strlen(string)-1;
   if(string[l]='\n') 
      string[l]='\0';
   else 
      string[l+1]='\0';
}

void recv_response(int sock, Response *response){
    char buffer[BSIZE];
    int bytes_read;

    bytes_read = read(sock, buffer, BSIZE);
    if(bytes_read <= BSIZE){
        parse_response(buffer, response);
    }
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