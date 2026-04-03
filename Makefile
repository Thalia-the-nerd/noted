all: noted

noted: main.c
	gcc main.c -o noted -lsodium -Wall

install: noted
	install -Dm755 noted /usr/local/bin/noted

clean:
	rm -f noted
