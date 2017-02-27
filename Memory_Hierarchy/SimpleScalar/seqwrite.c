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
	int j, l, iterations = 100;

	struct seq_test_data *seq_test_array = malloc(sizeof(struct seq_test_data)*array_sizes);   //to create array size number of elements
	
	for(l = 0; l < iterations; l++)			//accessing each block for iterations number of times
		for(j = 0; j < array_sizes; j++)		//writing data into one index of data array of array of elements sequentially
				seq_test_array[j].seqt_data[0] = 1;
	
	free(seq_test_array);	//free the array of blocks	
	return 0;
}
