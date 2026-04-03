all: noted

noted: main.c
	gcc main.c -o noted -lsodium -Wall

clean:
	rm -f noted
