#include "csapp.h"
#include <ctype.h>


int val_input(char *c){
    for(int i =0;i<6;i++){
        if(!isalnum(c[i])){
            return 0;
        }
    }
    return c[6]=='\n';
}

int val_ip(char *c){
    char ip[4][4];
    int n=0;
    int dots=0;
    
    for(int i =0;i<4;i++){
        ip[i][0]='\0';
    }
    
    while(*(c)){
        if(!isdigit(*c) && *c!='.' && *(c)!='\n'){
            return 0;
        }
        if(isdigit(*c)){
            n=0;
            while(isdigit(*c)){
                if(n<3){
                    strncat(ip[dots],c,1);
                }else{
                    return 0;
                }
                n++;
                c++;
            }
            ip[dots][3]='\0';
            if((*c !='.' && *c!='\n') || atoi(ip[dots])>255){
                return 0;
            }
        }
        while(*c=='.' || *c=='\n'){
            dots++;
            if(dots>4){
                return 0;
            }
            c++;
        }
    } 
    return dots==4 && strlen(ip[3])>0;
}

int main(int argc, char **argv) 
{
    int clientfd;
    char host[18], buf[8];
    rio_t rio;
    host[0]='\0';
    
    for(int i = 0;i<8;i++) buf[i] = '\0';
    
    if(argc==2){
        
        strcpy(host,argv[1]);
        strcat(host,"\n");
    }
    while(!val_ip(host)){
        printf("Input the server ip: ");
        Fgets(host, 18, stdin);  
        
        if (!strchr(host, '\n')) {
            int ch;
            while (((ch = getchar()) != EOF) && (ch != '\n'));
        }
        
    }
    
    clientfd = Open_clientfd(host, "1212");
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, 8, stdin) != NULL) {
        if(val_input(buf)){
            Rio_writen(clientfd, buf, strlen(buf));
            Rio_readlineb(&rio, buf, MAXLINE);
            Fputs(buf, stdout);
        }else{
            if (!strchr(buf, '\n')) {
                int ch;
                while (((ch = getchar()) != EOF) && (ch != '\n'));
            }
        }

    }
    Close(clientfd);
    exit(0);
}



