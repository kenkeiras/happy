CFLAGS=-ggdb -Wall -Werror -O3
CC=gcc

all: | bin obj bin/happy

bin/happy: obj/happy.o obj/model.o obj/transform.o
	$(CC) $(CFLAGS) -lm -o $@ $+

obj/happy.o : src/happy.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/model.o: src/lang-model/model.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/transform.o: src/transform-model/transform.c
	$(CC) $(CFLAGS) -c -o $@ $<

bin:
	mkdir bin || true

obj:
	mkdir obj || true

clean:
	rm -Rf bin/ obj/
