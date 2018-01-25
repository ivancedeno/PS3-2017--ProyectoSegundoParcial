#include "csapp.h"
#include "hash_table.h"
#include <string.h>
#include <signal.h>

#define SIZE 31
#define COUNT_MAX 10


Hasht * read_ht(char * filename);
static Hasht *ht;

int main(int argc, char **argv){
    ht=read_ht("hash.xml");
    if(ht==NULL){
        ht = new_ht(31);
    }else{
        print_ht(ht);
    }
    add_ht(ht,"asdasd");
    write_ht(ht,"hash.xml",1);

    exit(0);
}

Hasht * read_ht(char * filename){
    int f = open(filename, O_RDONLY,0);

    if(f<0)
        return NULL;
    rio_t rio;
    char line[MAXLINE]; 
    rio_readinitb(&rio,f);
    if(rio_readlineb(&rio, line , MAXLINE)<0){
        return NULL;
    }
    if(strcmp(line,"<table>\n")!=0){
        return NULL;
    }
    if(rio_readlineb(&rio, line , MAXLINE)<0){
        return NULL;
    }

    if(strncmp(line,"\t<size>",7)!=0){
        return NULL;
    }
    unsigned long size = atoi(line+7);
    Hasht *ht = new_ht(size);


    if(rio_readlineb(&rio, line , MAXLINE)<0){
        return NULL;
    }

    if(strcmp(line,"\t<contents>\n")!=0){
        return NULL;
    }
    ssize_t readed = rio_readlineb(&rio, line , MAXLINE);
    while(strcmp(line,"\t</contents>\n")!=0 && readed>0){
        if(strncmp(line,"\t\t<key>",7)==0 && strlen(line)==20 && strcmp(line+13,"</key>\n")==0){
            line[13]='\0';
            add_ht(ht,line+7);
        }

        readed = rio_readlineb(&rio, line , MAXLINE);
    }
    if(readed<0){
        return NULL;
    }
    if(rio_readlineb(&rio, line , MAXLINE)<0 || strcmp(line,"</table>\n")!=0){
        return NULL;
    }

    return ht;
    
}
