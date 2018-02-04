#include "csapp.h"
#include "hash_table.h"
#include <string.h>
#include <signal.h>
#include "config_reader.h"
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>


#define SIZE 31
#define COUNT_MAX 10
#define DEF_CONFIG_FILE "conf.cfg"
#define DEF_XML_FILE "hash.xml"
/* Se definen nombres por default de ciertos archivos
 * */


#define GET "GET "
#define IST "INSERT "
#define RM "REMOVE "

/*	
 * Estos define solo son para ahorrarse de escribir el la palabra completa con comillas
 * 
 * */

static volatile Hasht *ht;
static volatile int modified=0;
static volatile int running=1;
static sem_t mutex; 
static struct config *conf;
static pthread_t t_writer;
/*se definen variables globales para que los threads puedan acceder a ellas, modified es para notificar
 * que la hash table a sido modificada, asi evitar tener que escribir el archivo cuando no es necesario
 * running es para el thread que escribe archivos, notificarle que ya se debe detener
 * mutex es el semaforo para controlar el acceso a la hash table
 * conf, es una estructura en que se define la informacion leia de archivo de configuracion
 * t_writer es el thread que escribe la hashtable
 *  
 * */


struct info{
    char *name;
    int *connfd;

};

/*
 * Estructura que define la informacion del cliente al que nos hemos conectado, su nombre y el file descriptor
 * del socket
 * 
 * */


Hasht * read_ht(char * filename);
static void load_config(char *filename);
void * handle_client(void *);
void *thread_xml(void *v);

/*
Declaracion de funciones a usar en el programa
*/



void exit_handler(int i){

    running=0;
    pthread_join(t_writer, NULL);

    sem_wait(&mutex);
    if(modified){
    	write_ht((Hasht *) ht,conf->xml_file);
    }
    destroy_ht((Hasht *) ht);
    free(conf->xml_file);
    free(conf->port);
    free(conf);
    exit(0);
    sem_post(&mutex);
}
/* Funcion handler de en caso de señales sigint y sigkill, se cambia el estado de running para detener el thread
 * que escribe el archivo, se espera a que este thread termine con join
 * si existen otros threads modificando el hashtable se espera con sem_wait, si no lo existen o ya se desocupa uno, se bloquea el semaforo para que nadie maspueda modificar el hash table
 * si es que se modifico la hash table (o el thread que escribe el hash table no alcanzo a escribir los cambios)
 * se modifica el archivo xml del hash table
 * se libera la memoria de las estructuras utilizadas y se finaliza el programa
 * 
 * 
 * */


int main(int argc, char **argv){


    signal(SIGINT, exit_handler);
    signal(SIGKILL, exit_handler);
	
	/*
	 * se instalan las señales manejadoras de señales
	 * 
	 * 
	 * 
	 * 
	 * */
	
    char *filename=NULL;
    int opt;
	
	/*getopt es una funcion para leer los argumentos que se envian al programa,
	 * en este caso se dice que puede recibir un argumento -c el cual debe tener un parametro (indicado por :)
	 * si el parametro no necesita argumentos entonce no se coloca los :
	 * devuelve-1 cuando ya no se encuentran mas opciones
	 */
    while((opt = getopt(argc,argv,"c:"))!=-1){
    	switch(opt){
			/*
			 * Si se recibe el parametro entonces el nombre del archivo a leer se coloca al argumento
			 * */
    		case 'c':
    			filename = optarg;
    			break;
    		default:
    			break;
    	}
    }
    /*
	 * Si se consiguio un filename se abre ese archivo
	 * sino se usa el default name
	 *
	 */
	  
    if(filename!=NULL){
    	load_config(filename);
    }else{
    	load_config("conf.cfg");
    }
    //se inicializa el semaforo 
    sem_init(&mutex, 0, 1);
    
	//se crea el thread que escribe el xml
    pthread_create(&t_writer, NULL, thread_xml, conf);
    int listenfd, connfd;
    //Para socket que escucha y el que acepta la conexion
    socklen_t clientlen;

    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    /*	
	 * se define el port para escuchar conexiones
	 * */
    listenfd = Open_listenfd(conf->port);
    while (1) {

        clientlen = sizeof(struct sockaddr_storage); 
        /*
        la funcion accept bloquea el proceso hasta que se acepte una conexion

        */
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        //se acepta la conexion de un cliente
        
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                    client_port, MAXLINE, 0);
        //se obtiene el nombre del cliente
        /*
        Antes se usaba el nombre del cliente para indicar quien se conecto
        pero ya no hace falta porque el proceso de ver "silencioso" cuando esta en 
        background

        */

        struct info *inf = (struct info *) malloc(sizeof(struct info));
        /*
        se crea la estructura que obtiene la informacion de la nueva conexion
		el hostname tambien esta incluido aunque no es usado en esta implementacion
		(por si acaso)
		
        */
        inf->connfd = (int *) malloc(sizeof(int));
        /*
		como se trabajan con threads se deben crear variables propias para ellos
		primero con malloc, y luego se le asigna el valor de connfd
		*/

        *(inf->connfd) = connfd;

        inf->name = strdup(client_hostname);
        //printf("Connected to (%s, %s)\n", client_hostname, client_port);
        
        pthread_t tclient;
        //Se crea el thread y se asigna la funcion  que debe ejecutar
        /*
        handle_clint y se envia el pointer a la estructura de informacion
        */
        pthread_create(&tclient, NULL, handle_client, inf);
        
    }
    exit(0);
}



Hasht * read_ht(char * filename){



	/*
	Para leer el hashtable, se abre un archivo y se usan las funciones rio para
	leer linea a linea.
	en C los statements en un if se ejecutan de izquierda a derecha siempre
	entonces, debemos asegurar que se cumplan ciertas condiciones para que la lectura
	sea valida

	en estos casos, que se pueda leer una linea, luego que esa linea tenga cierto valor
	y asi para varias cosas, en caso que ocurra algo inesperado se devuelve null
	indicado un error en el archivo

	*/

    int f = open(filename, O_RDONLY,0);

    if(f<0)
        return NULL;
    rio_t rio;
    char line[MAXLINE]; 
    rio_readinitb(&rio,f);
    if(rio_readlineb(&rio, line , MAXLINE)<0 
        || strcmp(line,"<table>\n")!=0 
        || rio_readlineb(&rio, line , MAXLINE)<0 
        || strncmp(line,"\t<size>",7)!=0){
        return NULL;
    }
    /*
	si es que es que el proceso fue exitoso, la ultima linea leida representa el size de la tabla
	con la funcion atoi se lee un char y se obtiene el numero que tenga, cuando encuentre
	un caracter que no es un numero devuelve la cantidad leida hasta ese momento
	se le suma 7 a la lina para mover el puntero
	\t<size>number
	--------^
	existen 7 caracteres hasta llegar al numero
    */
    unsigned long size = atoi(line+7);
    


    if(rio_readlineb(&rio, line , MAXLINE)<0 || strcmp(line,"\t<contents>\n")!=0){
        return NULL;
    }
    Hasht *ht = new_ht(size);
    //se crea el hashtable y se empiezan a leer los contenidos
    ssize_t readed = rio_readlineb(&rio, line , MAXLINE);
    /*mientras la linea leida no sea el final de las llaves del xml
    y se haya leido más de 1 byte

    */
    while(strcmp(line,"\t</contents>\n")!=0 && readed>0){
    	/*
		se hacen validaciones con respecto a lo leido
		y si esta correcto se agregan elementos a la hashtable

   		*/
        if(strncmp(line,"\t\t<key>",7)==0 && strlen(line)==20 && strcmp(line+13,"</key>\n")==0){
            line[13]='\0';
            add_ht((Hasht *) ht,line+7);
        }

        readed = rio_readlineb(&rio, line , MAXLINE);
    }
    /*
    en caso de que el while termino porque se llego al final del archivo y no se encontro
    el final del contenido
    */ 
    if(readed<0){
        destroy_ht((Hasht *) ht);
        return NULL;
    }
    //finalizando de leer el archivo
    if(rio_readlineb(&rio, line , MAXLINE)<0 || strcmp(line,"</table>\n")!=0){
        destroy_ht((Hasht *) ht);
        return NULL;
    }
    //retorna la hashtable
    return ht;
    
}

static void load_config(char *filename){
	/*
	se lee la estructura de configuracion
	en caso de que exista algun error se libera memoria en caso de haberla reservado
	y se cierra el programa
	*/
    conf = read_config_file(filename);
    if(conf==NULL){
        fprintf(stderr,"Configuration file not found\n");
        exit(-1);
    }else if(conf->port==NULL || conf->xml_file==NULL || conf->period <1 || conf->table_size<1){
        fprintf(stderr,"Configuration file has an error and could not be loaded\n");
        if(conf->port!=NULL){
        	free(conf->port);
        }
        if(conf->xml_file!=NULL){
        	free(conf->xml_file);
        }
        free(conf);
        exit(-1);
    }
    //si todo fue con exito se lee la hahstable del archivo
    //si no existe el archivo o tiene un error se crea una nueva
    //con el tamaño que estaba en el archivo de configuracion

    ht=read_ht(conf->xml_file);
    if(ht==NULL){
        ht = new_ht(conf->table_size);
    }
}

void * handle_client(void *v){
	/*
	Los threads que se crean para manejar clientes se les hace detach para
	que cuando terminen de atender al cliente liberen sus recursos automaticamente	
	*/
    pthread_detach(pthread_self());
    //Se recupera la informacion de la estructura enviada
    struct info *inf= (struct info *) v;
    size_t n; 
    char buf[MAXLINE];
    char response[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, *(inf->connfd));
    while(running && (n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { 
        //printf("server received %d bytes\n", (int)n);
        /*
		Mientras el programa siga corriendo y se lea una linea de un cliente
		como las acciones del cliente hacen cambios en la hashtable se usa un semaforo
		para controlar que no existan inserciones, busquedas y eliminaciones a la vez
		
        */
        sem_wait(&mutex);
        if(strncmp(buf,GET,strlen(GET))==0){
        	strcpy(response,(buf+strlen(GET)));
        	response[6]='\0';
        	strcat(response,(get_ht((Hasht *)ht,response)==NULL)?",NO\n":",SI\n");

        }else if(strncmp(buf,IST,strlen(IST))==0){
        	modified=1;
        	strcpy(response,(buf+strlen(IST)));
        	response[6]='\0';
        	strcat(response,(add_ht((Hasht *) ht,response))?",Added\n":",Already in the hash table\n");
        }else if(strncmp(buf,RM,strlen(RM))==0){
        	modified=1;
			strcpy(response,(buf+strlen(RM)));
        	response[6]='\0';
        	char *tmp =remove_ht((Hasht *) ht,response);
        	strcat(response,(tmp!=NULL)?",Removed\n":",Not in the hash table\n");
        	if(tmp!=NULL){
        		free(tmp);
        	}
        	
        }else{
        	strcpy(response,"Invalid operation\n");
        }
        sem_post(&mutex);
        Rio_writen(*inf->connfd, response,strlen(response));
        
    }
    /*
    esto es en caso que se quiera mostrar la informacion de la hashtable luego de manjear al cliente
    sem_wait(&mutex);
    printf("Keys in the hash table: \n");
    print_ht((Hasht *) ht);
    sem_post(&mutex);
    */
    //printf("Disconnected from: %s\n",inf->name);
    /*
    se libera la memoria de las estructuras usadas
    */ 
    Close(*inf->connfd);
    free(inf->connfd);
    free(inf->name);
    free(inf);
    return NULL;
    
}

void *thread_xml(void *v){
	/*
	se recupera la estructura enviada y se inicializa un contador en cerp
	
	*/
    struct config *conf = (struct config *) v;
    int count=0;
    while(running){//mientras el programa siga corriendo
        if(count<conf->period){
        	//si el conteo es menor que el periodo, se aumenta en uno y se duerme 1 segundo
            count++;
            sleep(1);
        }else{
        	//caso contrario se reinicia el contador
        	count=0;
        	if(modified){//si es que la hashtable fue modificada se escribe otra vez el archivo xml
        		sem_wait(&mutex);
           		write_ht((Hasht *) ht,conf->xml_file);
            	modified=0;
				sem_post(&mutex);
        	}
        	
			
        }
        
        
    }
	return NULL;
}
