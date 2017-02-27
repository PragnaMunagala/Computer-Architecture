#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

/* structure of cache line size for sequential memory access */
struct seq_test_data{
	int seqt_data[16];
};

/*************** Main Program ****************/	
int main(int argc, char* argv[]){
	/* to create different random values each time*/
	srand(time(NULL));

	/* Array to hold different values of array sizes read from command line */
	int array_sizes = atoi(argv[1]);
	int j, k, l, iterations = 100;

	/* to hold the read value from memory */
	register int read_val = 0;
	struct seq_test_data *seq_test_array = malloc(sizeof(struct seq_test_data)*array_sizes);	 //to create array size number of elements

	/* to write some value before reading to the data array of each sequential element */
	for(l = 0;l < array_sizes; l++){
		for(k = 0; k < 16; k++)
			seq_test_array[l].seqt_data[k] = 0;
	}
			
	for(l = 0; l < iterations; l++)		//accessing each block for iterations number of times
		for(j = 0; j < array_sizes; j++)		//accessing data from each element of array of elements sequentially
			read_val = seq_test_array[j].seqt_data[0];
	
	/* random check operation using read value to bypass compiler optimization. The idea here is if the read_val is not used in the program
	after it is set, then compiler MAY optimize and may not execute the read loop.*/
	if(seq_test_array[j-1].seqt_data[0] == read_val - 1 )
	{
		asm volatile("nop");
	}
	free(seq_test_array);		//free the array of blocks		
	return 0;
}
