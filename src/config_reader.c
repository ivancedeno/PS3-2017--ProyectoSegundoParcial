#include <stdio.h> 
#include "csapp.h"
#include <stdlib.h>
#include "config_reader.h"

struct config *read_config_file(char *filename){
	rio_t rio;
	
	int fd = open(filename,O_RDONLY,0);
	if(fd<0){
		return NULL;
	}
	/*

	proceso muy similar al de leer la hashtable se leen lineas y si se ecuentra algo relevante se lo lee
	y asigna a la estructura que contiene la informacion
	*/

	rio_readinitb(&rio, fd);
	struct config *con = (struct config *) malloc(sizeof(struct config));
	con->port = NULL;
	con->xml_file = NULL;
	con->period=-1;
	con->table_size=-1;
	char buffer[MAXLINE];
	while(rio_readlineb(&rio,buffer,MAXLINE)>0){
		if(strncmp(buffer,"puerto=",7)==0 && strlen(buffer)>8){
			buffer[strlen(buffer)-1]='\0';
			con->port = strdup(buffer+strlen("puerto="));
		}else if(strncmp(buffer,"archivo_tabla=",strlen("archivo_tabla="))==0 && strlen(buffer)>strlen("archivo_tabla=")){
			buffer[strlen(buffer)-1]='\0';
			con->xml_file=strdup(buffer+strlen("archivo_tabla="));

		}else if(strncmp(buffer,"periodo_archivo=",strlen("periodo_archivo="))==0 && strlen(buffer)>strlen("periodo_archivo=")){
			con->period = atoi(buffer+strlen("periodo_archivo="));
		}else if(strncmp(buffer,"tamano_tabla=",strlen("tamano_tabla="))==0 && strlen(buffer)>strlen("tamano_tabla=")){
			con->table_size = atoi(buffer+strlen("tamano_tabla="));
		}
	}

	return con;
}