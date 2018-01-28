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

#define GET "GET "
#define IST "INSERT "
#define RM "REMOVE "


static volatile Hasht *ht;
static volatile int modified=0;
static volatile int running=1;
static sem_t mutex; 
static struct config *conf;
static pthread_t t_writer;

struct info{
    char *name;
    int *connfd;

};


Hasht * read_ht(char * filename);
static void load_config(char *filename);
void * handle_client(void *);
void *thread_xml(void *v);

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

int main(int argc, char **argv){


    signal(SIGINT, exit_handler);
    signal(SIGKILL, exit_handler);
    char *filename=NULL;
    int opt;
    while((opt = getopt(argc,argv,"c:"))!=-1){
    	switch(opt){
    		case 'c':
    			filename = optarg;
    			break;
    		default:
    			break;
    	}
    }
    if(filename!=NULL){
    	load_config(filename);
    }else{
    	load_config("conf.cfg");
    }
    sem_init(&mutex, 0, 1);
    

    pthread_create(&t_writer, NULL, thread_xml, conf);
    int listenfd, connfd;

    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    
    listenfd = Open_listenfd(conf->port);
    while (1) {

        clientlen = sizeof(struct sockaddr_storage); 
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                    client_port, MAXLINE, 0);


        struct info *inf = (struct info *) malloc(sizeof(struct info));
        inf->connfd = (int *) malloc(sizeof(int));
        *(inf->connfd) = connfd;
        inf->name = strdup(client_hostname);
        //printf("Connected to (%s, %s)\n", client_hostname, client_port);
        
        pthread_t tclient;
        pthread_create(&tclient, NULL, handle_client, inf);
        
    }
    exit(0);
}



Hasht * read_ht(char * filename){
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
    unsigned long size = atoi(line+7);
    


    if(rio_readlineb(&rio, line , MAXLINE)<0 || strcmp(line,"\t<contents>\n")!=0){
        return NULL;
    }
    Hasht *ht = new_ht(size);
    ssize_t readed = rio_readlineb(&rio, line , MAXLINE);
    while(strcmp(line,"\t</contents>\n")!=0 && readed>0){
        if(strncmp(line,"\t\t<key>",7)==0 && strlen(line)==20 && strcmp(line+13,"</key>\n")==0){
            line[13]='\0';
            add_ht((Hasht *) ht,line+7);
        }

        readed = rio_readlineb(&rio, line , MAXLINE);
    }
    if(readed<0){
        destroy_ht((Hasht *) ht);
        return NULL;
    }
    if(rio_readlineb(&rio, line , MAXLINE)<0 || strcmp(line,"</table>\n")!=0){
        destroy_ht((Hasht *) ht);
        return NULL;
    }

    return ht;
    
}

static void load_config(char *filename){
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

    ht=read_ht(conf->xml_file);
    if(ht==NULL){
        ht = new_ht(conf->table_size);
    }
}

void * handle_client(void *v){
    pthread_detach(pthread_self());
    struct info *inf= (struct info *) v;
    size_t n; 
    char buf[MAXLINE];
    char response[MAXLINE];
    rio_t rio;
    Rio_readinitb(&rio, *(inf->connfd));
    while(running && (n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { 
        //printf("server received %d bytes\n", (int)n);
        
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
    sem_wait(&mutex);
    printf("Keys in the hash table: \n");
    print_ht((Hasht *) ht);
    sem_post(&mutex);
    */
    //printf("Disconnected from: %s\n",inf->name);
    Close(*inf->connfd);
    free(inf->connfd);
    free(inf->name);
    free(inf);
    return NULL;
    
}

void *thread_xml(void *v){
    struct config *conf = (struct config *) v;
    int count=0;
    while(running){
        if(count<conf->period){
            count++;
            sleep(1);
        }else{
        	count=0;
        	if(modified){
        		sem_wait(&mutex);
           		write_ht((Hasht *) ht,conf->xml_file);
            	modified=0;
				sem_post(&mutex);
        	}
        	
			
        }
        
        
    }
	return NULL;
}
