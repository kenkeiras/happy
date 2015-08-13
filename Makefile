CFLAGS=-ggdb -Wall -Werror -O3 -pg
CC=gcc

all: | bin obj bin/happy

bin/happy: obj/happy.o obj/model.o obj/transform.o obj/hash_table.o obj/linked_list.o
	$(CC) $(CFLAGS) -lm -o $@ $+

obj/happy.o : src/happy.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/model.o: src/lang-model/model.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/transform.o: src/transform-model/transform.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/hash_table.o: src/ht/hash_table.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/linked_list.o: src/ht/linked_list.c
	$(CC) $(CFLAGS) -c -o $@ $<

bin:
	mkdir bin || true

obj:
	mkdir obj || true

clean:
	rm -Rf bin/ obj/
