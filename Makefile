all: bin
	$(CC) hidraw-poke.c --std=gnu99 -o bin/hidraw-poke

clean:
	rm -r bin

bin:
	mkdir bin

