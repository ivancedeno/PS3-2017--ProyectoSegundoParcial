all: library server client
library: lib/hash_ps3.a


lib/hash_ps3.a: obj/hash_table.o include/hash_table.h
	mkdir -p lib
	ar rcs lib/hash_ps3.a obj/hash_table.o

obj/config_reader.o: src/config_reader.c include/config_reader.h
	mkdir -p obj
	gcc -Wall -O2 -c src/config_reader.c -I./include -o $@

obj/csapp.o: src/csapp.c include/csapp.h
	mkdir -p obj
	gcc -Wall -O2 -c src/csapp.c -I./include -o $@

client: obj/client.o obj/csapp.o
	gcc -Wall -O2 obj/client.o obj/csapp.o -lpthread -o $@

obj/client.o: src/client.c
	mkdir -p obj
	gcc -Wall -O2 -c $^ -o $@ -I./include
	
server: obj/server.o obj/csapp.o obj/config_reader.o lib/hash_ps3.a 
	gcc -Wall -O2 $^ -o $@ -lpthread

obj/server.o: src/server.c
	mkdir -p obj
	gcc -Wall -O2 -c $^ -o $@ -I./include
	
obj/hash_table.o: src/hash_table.c include/hash_table.h
	mkdir -p obj
	gcc -Wall -O2 -c -I./include src/hash_table.c -o $@


	
