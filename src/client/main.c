#include "header.h"

int sock_opd;

int main(int argc, char* argv[])
{   
    struct sockaddr_in sv_adress;
    char command[256];
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
    if (inet_aton(argv[1], (struct in_addr *)(&sv_adress.sin_addr.s_addr)) == 0 )
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
        while(1){
            Command *cmd = malloc(sizeof(Command));
            fflush(stdin);
            fgets(command, sizeof(command), stdin);
            clean_string(command);

            parse_command(command, cmd);
            
            code = execute_command(cmd, sock_opd);
        }
    }
    //Close socket
    close(sock_opd);

    return 0;
}

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
   if(string[l]=='\n') 
      string[l]='\0';
   else 
      string[l+1]='\0';
}

void recv_response(int sock, Response *response){
    char buffer[BSIZE];
    int bytes_read;

    memset(buffer, 0, BSIZE);
    bytes_read = read(sock, buffer, BSIZE);
    if(bytes_read <= BSIZE){
        parse_response(buffer, response);
    }
	printf("%s", buffer);
	memset(buffer, 0, BSIZE);
}

void send_file(int sock, char* filename){
    int fd;
    int sent_bytes = 0;
    struct stat file_stat;
    off_t offset = 0; 
    int remain_data;

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
            fprintf(stdout, "Client sent %d bytes from file's data, offset is now : %d and remaining data = %d\n", sent_bytes, (int)offset, remain_data);
    }
    
    close(sock);
}
