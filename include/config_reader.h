#ifndef CONFRE_H
#define CONFRE_H

struct config {
	char *port;
	char *xml_file;
	int period;
	int table_size;
};
struct config *read_config_file(char *filename);
#endif
