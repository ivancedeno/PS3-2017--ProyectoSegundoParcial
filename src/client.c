#include "csapp.h"
#include <ctype.h>
#include <unistd.h>

/*
definiciones solo para no escribir las palabras completas
*/
#define GET "GET "
#define IST "INSERT "
#define RM "REMOVE "
/*
funcion para validar quelo ingresado sea alguna operacion valida
luego se mueve el puntero para verificar que lo que le sigue es una key
de 6 caracteres alfanumericos y luego que se encuentre el caracter
de salto de linea

*/
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


/*
para validar que se ingreso una ip valida
la ip tiene 3 puntos peroen esta validacion se aumenta 1
por conveniencia de la validacion
la variable ip representa un array que contiene arrays
para poder representar cada numero como char por separado
n es la cantidad de digitos leidos separado por puntos
siempre debe ser entre 1 y 3

*/
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
    /*
    variables para guardar al host, un buffer para guardar lo que
    escribe el usuario y el puerto*/
    while((opt=getopt(argc,argv,"s:p:"))!=-1){
        switch(opt){
            case 's':
                strcpy(host,optarg);
                /*
                en la validacionse toma en cuenta el \n ya que se estaba ingresando
                por teclado
                ahora no se concatena el \n porque no se esta validando
                ademas que causa problemas para realizar la conexion y deberia ser eliminado

                */
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
    // se inicializa el buffer vacio

    /*
    por si se quiere dejar que el usuario ingrese la ip del servidor por teclado
    pero tambien hace falta el puerto, por eso mas conveniente dejarlo que se ingrese
    por argumento del programa
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
            /*
            si es que lo ingresado no es valido
            se limpia el buffer y posible texto que haya quedado en espera por solo leer
            15 caracteres a la vez
            */
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



