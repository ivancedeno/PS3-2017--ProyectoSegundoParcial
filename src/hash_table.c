#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "hash_table.h"
#include <sys/types.h>
#include <sys/stat.h> 

#define PRIME 1307


/*
According to a simulation of expected input and their resulted hash_code
we found that there are about ~1400 different posible hashes (before applying mod to a prime number for size) for the almost 5.4x10^10 posible permutations of 6 characters(including lowercase, uppercase and numbers) which means that our hash function is highly inefficient, even if our prime number-size was big it wouldn't mean any benefit, so we have decided to use 1307 as the size of the table
*/




void mem_error(){
    printf("Problem allocating memory\n");
    exit(1);
} 


int hash_code(char *c, int size){
	unsigned int code=0;
	int i=1;
	while(*(c)!='\0'){
		code+=i*(*c);		
		c++;
		i++;
	}
	return code%size;
}


Hasht *new_ht(int size){

	Hasht *h_t;
	if((h_t=(Hasht *) malloc(sizeof(Hasht)))==NULL){
        mem_error();
	}

	if((h_t->table=(ENTRY **)malloc(sizeof(ENTRY *)*size))==NULL){
        mem_error();
	}
	for(unsigned long i=0;i<size;i++){
		h_t->table[i]=NULL;
	}
	h_t->size=size;
	h_t->n_size=0;
	return h_t;
	
}

ENTRY *create_entry(char *n, int size){
	ENTRY *en = (ENTRY *) malloc(sizeof(ENTRY));
	if(en==NULL){
		return NULL;
	}
	en->key=hash_code(n,size);
    en->data=strdup(n);
	en->next=NULL;
	return en;
	
}


int add_entry(Hasht *h_t, ENTRY *en){
	ENTRY *f=h_t->table[en->key];
	if(f==NULL){
		h_t->table[en->key]=en;
	}else{
        if(strcmp(f->data,en->data)==0){
            free(en->data);
            free(en);
            return 0;
        }
		while((f->next)!=NULL){
			if(f!=NULL){
				if(strcmp(f->data,en->data)==0){
                    free(en->data);
                    free(en);
                    return 0;
				}
				f=f->next;
			}
		}
		
        f->next=en;
	}
	h_t->n_size++;
	return 1;
}

int add_ht(Hasht *h_t, char *n){
	ENTRY *en=create_entry(n, h_t->size);
	if(en!=NULL){
		return add_entry(h_t,en);
	}else{
		mem_error();
	}
	return 0;
}




void print_ht(Hasht *h_t){
	int i =0;

	while(i<(h_t->size)){
        
		if(h_t->table[i]!=NULL){
			printf("%s\n",h_t->table[i]->data);
			if(h_t->table[i]->next!=NULL){
				ENTRY *tmp =h_t->table[i]->next;
				while(tmp!=NULL){
					printf("%s\n",tmp->data);
					tmp=tmp->next;
				}
			}
		}
		i++;
	}
}


void *get_ht(Hasht *t, char *c){
	ENTRY *en=t->table[hash_code(c,t->size)];
	if(en!=NULL){
		while(en->next!=NULL && strcmp(en->data,c)!=0){
			en=en->next;
		}
		if(en!=NULL && strcmp(en->data,c)==0){
			return en->data;
		}
	}
	return NULL;
}



void *remove_ht(Hasht *h_t, char *c){
	unsigned int i =hash_code(c,h_t->size);
	ENTRY *f=h_t->table[i];
	const int r = strcmp(f->data,c);
	char *n = NULL;
	if(r!=0){
		
		while(((f->next)!=NULL && strcmp(f->next->data,c)!=0)){
			f=f->next;
		}
		if(f!=NULL && (f->next)!=NULL && strcmp(f->next->data,c)==0){
			ENTRY *b = f->next;
			if(b->next!=NULL){
				f->next=b->next;
			}else{
				f->next=NULL;
			}
			h_t->n_size--;
			free(b->data);
			free(b);
		}
	}else if(r==0){
		
		if(f->next != NULL){
			h_t->table[i]=f->next;
		}else{
			h_t->table[i]=NULL;
		}
		h_t->n_size--;
		free(f->data);
		free(f);
	}
	return n;
}

void destroy_ht(Hasht *t){
    ENTRY *en=NULL;
    ENTRY *tmp=NULL;
    for(int i =0; i< t->size;i++){
        if(t->table[i]!=NULL){
            free(t->table[i]->data);
            t->table[i]->data=NULL;
            en=t->table[i]->next;
            while(en!=NULL){
                free(en->data);
                en->data=NULL;
                tmp=en->next;
                free(en);
                en=tmp;
            }
            tmp=NULL;
            t->table[i]->next=NULL;
            free(t->table[i]);
            t->table[i]=NULL;
        }
    }
    free(t->table);
    free(t);
}

ENTRY* getNext(Hasht * ht, ENTRY *en, int *current){
	if(*current< ht->size){
		if(en==NULL){
			while(*current < ht->size && ht->table[*current]==NULL){
				(*current)++;
			}
			if(*current< ht->size)
				return ht->table[*current];
			else
				return NULL;
		}else if(en->next!=NULL){
			return en->next;
		}else{
			(*current)++;
			return getNext(ht,NULL,current);
		}
	}
	return NULL;

}

void write_ht(Hasht *ht, char * filename, int modified){
	int current=0;
	if(!modified)
		return;
	umask(S_IWGRP|S_IWOTH);
	FILE *f = fopen(filename,"w");
	if(f==NULL){
		fprintf(stderr,"Can' write hash table xml file\n");
		exit(-1);
	}
	char *header = "<table>\n\t<size>";
	char buffer[500];
	sprintf(buffer,"%ld",ht->size);
	fputs(header, f);


	header = "</size>\n\t<contents>";

	fputs(buffer,f);
	fputs(header,f);
	ENTRY *en = NULL;
	while((en=getNext(ht,en,&current))!=NULL){
		strcpy(buffer,"\n\t\t<key>");
		strcat(buffer,en->data);
		strcat(buffer,"</key>");
		fputs(buffer,f);
	}
	strcpy(buffer,"\n\t</contents>\n</table>\n");
	fputs(buffer,f);
	current=0;

	fflush(f);
	fclose(f);

}


