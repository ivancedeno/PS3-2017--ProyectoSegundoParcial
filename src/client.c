#include "csapp.h"
#include <ctype.h>
#include <unistd.h>


#define GET "GET "
#define IST "INSERT "
#define RM "REMOVE "

int val_input(char *c){
    if(strncmp(c,GET,strlen(GET))==0){
        c+=strlen(GET);
    }else if(strncmp(c,IST,strlen(IST))==0){
        c+=strlen(IST);
    }else if(strncmp(c,RM,strlen(RM))==0){
        c+=strlen(RM);
    }else{
        return 0;
    }
    for(int i =0;i<6;i++){
        if(!isalnum(c[i])){
            return 0;
        }
    }
    return c[6]=='\n';
}

int val_ip(char *c){
    if(strcmp(c,"localhost\n")==0){
        return 1;
    }
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

    int opt;
    char host[18], buf[15];
    char port[5];
    port[0]='\0';

    while((opt=getopt(argc,argv,"s:p:"))!=-1){
        switch(opt){
            case 's':
                strcpy(host,optarg);
                //strcat(host,"\n");
                break;
            case 'p':
                strcpy(port,optarg);
                break;
            default:
                break;
        }
    }



    int clientfd;

    rio_t rio;
    
    for(int i = 0;i<15;i++) buf[i] = '\0';
    /*
    while(!val_ip(host)){
        printf("Input the server ip: ");
        Fgets(host, 18, stdin);  
        
        if (!strchr(host, '\n')) {
            int ch;
            while (((ch = getchar()) != EOF) && (ch != '\n'));
        }
        
    }
    if (strchr(host, '\n')){
        *strchr(host, '\n')='\0';
    }
    */
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, 15, stdin) != NULL) {
        if(val_input(buf)){
            Rio_writen(clientfd, buf, strlen(buf));
            Rio_readlineb(&rio, buf, MAXLINE);
            Fputs(buf, stdout);
        }else{
            if (!strchr(buf, '\n')) {
                int ch;
                while (((ch = getchar()) != EOF) && (ch != '\n'));
            }
            printf("Invalid input\nValid operations are GET | INSERT | REMOVE <key> \nKeys must be 6 characters long\n");
        }

    }
    Close(clientfd);
    exit(0);
}



