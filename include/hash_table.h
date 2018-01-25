
#ifndef HASH_H
#define HASH_H
typedef struct entry_set{
	unsigned int key;
	char *data;
	struct entry_set *next;
} ENTRY;

typedef struct h_t{
	unsigned long size;
	unsigned long n_size;
	ENTRY **table;
} Hasht;


Hasht *new_ht(int);
int add_ht(Hasht *h_t, char *n);
void print_ht(Hasht *h_t);
void *get_ht(Hasht *t, char* c);
void *remove_ht(Hasht *t, char *c);
void destroy_ht(Hasht *t);
void write_ht(Hasht *ht, char * filename, int modified);

#endif
