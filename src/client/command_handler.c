#include "header.h"

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
    int socksend;
    char buff[BSIZE];
    char *cmdfn = "STOR %s";
    Response *response = malloc(sizeof(Response)); 
    ConnectionInfo *cifs = malloc(sizeof(ConnectionInfo));

    ftp_pasv(sock, cifs);
    sprintf(buff, cmdfn, filepath);
    send(sock, buff, sizeof(buff), 0);

    socksend = connect_to_server(cifs);

    send_file(socksend, filepath);

    recv_response(sock, response);
    recv_response(sock, response);
    return response->code;
}

int ftp_recv_file(int sock, char* filepath){
    int socksend, fdfile, pipefd[2], res;
    char buff[BSIZE];
    char *cmdfn = "RETR %s";
    Response *response = malloc(sizeof(Response)); 
    ConnectionInfo *cifs = malloc(sizeof(ConnectionInfo));

    ftp_pasv(sock, cifs);
    sprintf(buff, cmdfn, filepath);
    send(sock, buff, sizeof(buff), 0);

    recv_response(sock, response);

    if(response->code == 150){
        socksend = connect_to_server(cifs);
        FILE *fp = fopen(filepath,"w");
        fdfile = fileno(fp);
        if(pipe(pipefd)==-1)perror("ftp_stor: pipe");
           while ((res = splice(socksend, 0, pipefd[1], NULL, 8192, SPLICE_F_MORE | SPLICE_F_MOVE))>0){
               splice(pipefd[0], NULL, fdfile, 0, 8192, SPLICE_F_MORE | SPLICE_F_MOVE);
             }
    }       
    recv_response(sock, response);
    return response->code;
}

int ftp_remove_file(int sock, char* filepath){
    char buff[BSIZE];
    char *cmdfn = "DELE %s";
    Response *response = malloc(sizeof(Response)); 
    
    sprintf(buff, cmdfn, filepath);
    send(sock, buff, sizeof(buff), 0);
    
    recv_response(sock, response);
    return response->code;
}

int ftp_ls_firectory(int socket, char* path){
    int socksend, r;
    char buff[BSIZE];
    char *cmdfn = "LIST %s";
    Response *response = malloc(sizeof(Response)); 
    ConnectionInfo *cifs = malloc(sizeof(ConnectionInfo));
    
    ftp_pasv(socket, cifs);
    
    sprintf(buff, cmdfn, path);
    send(socket, buff, sizeof(buff), 0);

    socksend = connect_to_server(cifs);

    recv_response(socket, response);
    if(response->code == 150){
        memset(buff, 0, BSIZE);
        while((r = recv(socksend , buff , BSIZE, 0)) > 0)
        {
            printf("%s", buff);
        }
    }

    recv_response(socket, response);
    return response->code;
}

int ftp_admin_users(int socket, char* path){
    int socksend, r;
    char buff[BSIZE];
    char *cmdfn = "USERS";
    Response *response = malloc(sizeof(Response)); 
    ConnectionInfo *cifs = malloc(sizeof(ConnectionInfo));
    
    ftp_pasv(socket, cifs);
    
    sprintf(buff, cmdfn, path);
    send(socket, buff, sizeof(buff), 0);

    recv_response(socket, response);

    if(response->code == 150){
        memset(buff, 0, BSIZE);

        socksend = connect_to_server(cifs);

        while((r = recv(socksend , buff , BSIZE, 0)) > 0)
        {
            printf("%s", buff);
        }
    }else if(response->code > 500){
        return response->code;
    }

    recv_response(socket, response);
    return response->code;
}

int ftp_pasv(int sock, ConnectionInfo* cif){
    int ip[4], p1, p2, socksend;
    char cmd[4] = "PASV";
    Response *response = malloc(sizeof(Response)); 
    
    printf("%s\n", cmd);
    write(sock, cmd, 4);
    
    recv_response(sock, response);

    if(response->code == 227){
        sscanf(response->message,"%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d",&ip[0],&ip[1],&ip[2],&ip[3],&p1,&p2);
        
        cif->ip[0] = ip[0];
        cif->ip[1] = ip[1];
        cif->ip[2] = ip[2];
        cif->ip[3] = ip[3];
        cif->port = 256 * p1 + p2;
    }
    return response->code;
}

int ftp_pwd(int sock, char* args){
    char buff[BSIZE] = "PWD";
    Response *response = malloc(sizeof(Response)); 
    
    send(sock, buff, sizeof(buff), 0);
    
    recv_response(sock, response);
    return response->code;
}

int ftp_cwd(int sock, char* args){
    char buff[BSIZE];
    char *cmdfn = "CWD %s";
    Response *response = malloc(sizeof(Response)); 
    
    sprintf(buff, cmdfn, args);
    send(sock, buff, sizeof(buff), 0);
    
    recv_response(sock, response);
    return response->code;
}
int connect_to_server(ConnectionInfo *cif){
    int socksend;
    struct sockaddr_in sv_adress;
    char ipadr[17];
    sprintf(ipadr, "%d.%d.%d.%d\0", cif->ip[0],cif->ip[1],cif->ip[2],cif->ip[3]);

    socksend = socket(AF_INET, SOCK_STREAM , 0);
    if(socksend < 0){
        printf("Open socket error!\n");
        exit(0);
    }

    sv_adress.sin_family = AF_INET;
    sv_adress.sin_port = htons(cif->port);
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
    return socksend;
}

int lookup_cmd(char *cmd){
  const int cmdlist_count = sizeof(cmdlist_str)/sizeof(char *);
  return lookup(cmd, cmdlist_str, cmdlist_count);
}

int lookup(char *needle, const char **haystack, int count)
{
  int i;
  for(i=0;i<count; i++){
    if(strcmp(needle,haystack[i])==0)return i;
  }
  return -1;
}

int execute_command(Command *cmd, int sock){
	switch(lookup_cmd(cmd->command)){
	    case LIST: ftp_ls_firectory(sock, cmd->arg); break;
	    case RETR: ftp_recv_file(sock, cmd->arg); break;
	    case STOR: ftp_send_file(sock, cmd->arg); break;
	    case DELE: ftp_remove_file(sock, cmd->arg); break;
        case PWD: ftp_pwd(sock, cmd->arg); break;
        case CWD: ftp_cwd(sock, cmd->arg); break;
        case USERS: ftp_admin_users(sock, cmd->arg); break;
	    default: 
	      break;
	  }
}