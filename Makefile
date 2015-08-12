CFLAGS=-ggdb -Wall -Werror -O3

all: | bin obj bin/happy

bin/happy: obj/happy.o obj/model.o obj/transform.o
	gcc $(CFLAGS) -lm -o $@ $+

obj/happy.o : src/happy.c
	gcc $(CFLAGS) -c -o $@ $<

obj/model.o: src/lang-model/model.c
	gcc $(CFLAGS) -c -o $@ $<

obj/transform.o: src/transform-model/transform.c
	gcc $(CFLAGS) -c -o $@ $<

bin:
	mkdir bin || true

obj:
	mkdir obj || true

clean:
	rm -Rf bin/ obj/
