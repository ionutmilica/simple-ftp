#include "header.h"

int sock_opd;

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