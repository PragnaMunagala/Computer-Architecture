CFLAGS = -Wall
output: main.o
	gcc -O0 -o output main.o

main.o: main.c
	gcc ${CFLAGS} -c main.c
	
clean:
	rm -rf *.o *.ko output