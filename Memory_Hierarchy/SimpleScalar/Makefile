CFLAGS = -Wall
all: seqread1 seqwrite1 ranread1 ranwrite1

ranwrite1: ranwrite.o
	alpha-dec-osf4-gcc -O0 ranwrite.o -o ranwrite1

ranwrite.o: ranwrite.c
	alpha-dec-osf4-gcc ${CFLAGS} -c ranwrite.c

ranread1: ranread.o
	alpha-dec-osf4-gcc -O0 ranread.o -o ranread1

ranread.o: ranread.c
	alpha-dec-osf4-gcc ${CFLAGS} -c ranread.c

seqwrite1: seqwrite.o
	alpha-dec-osf4-gcc -O0 seqwrite.o -o seqwrite1

seqwrite.o: seqwrite.c
	alpha-dec-osf4-gcc ${CFLAGS} -c seqwrite.c

seqread1: seqread.o
	alpha-dec-osf4-gcc -O0 seqread.o -o seqread1

seqread.o: seqread.c
	alpha-dec-osf4-gcc ${CFLAGS} -c seqread.c
	
clean:
	rm -rf *.o *.ko seqread1 seqwrite1 ranread1 ranwrite1