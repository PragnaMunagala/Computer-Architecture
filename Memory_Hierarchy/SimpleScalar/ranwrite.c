#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

/* structure of cache line size for random memory access */
struct ran_test_data{
	int rant_data[12];
	int em_rasav;
	int dentlo_rasav;
	struct ran_test_data *ptr;
};

/*************** Main Program ****************/
int main(int argc, char* argv[]){
	/* to create different random values each time*/
	srand(time(NULL));

	/* Array to hold different values of array sizes read from command line */
	int array_sizes = atoi(argv[1]);
	int j, k=0, l, p = 0, previous_j, iterations = 100;

	//clock_t start, end;
	struct ran_test_data *ran_test_array = malloc(sizeof(struct ran_test_data)*array_sizes);	   //to create array size number of elements

	/* initialization of flags of each element */
	while(k < array_sizes)
	{
		ran_test_array[k].dentlo_rasav = -1;
		ran_test_array[k].em_rasav = -1;
		k++;
	}
	/************* Pointer Chasing ***************/
	k = 0;	
	while(k < array_sizes)
	{
		j = rand()%array_sizes;		//generating random number
		
		/* storing the location of randomly generated index element into first element of array */
		if (ran_test_array[0].em_rasav == -1)
		{
			ran_test_array[0].ptr = &ran_test_array[j];
			previous_j = j; 
			ran_test_array[0].em_rasav = j;
			ran_test_array[j].dentlo_rasav=0;
			k++;
			continue;	
		}
		if(ran_test_array[previous_j].em_rasav == -1)
		{
			if(ran_test_array[j].dentlo_rasav == -1)
			{
				ran_test_array[previous_j].ptr = &ran_test_array[j];
				ran_test_array[previous_j].em_rasav = j;
				ran_test_array[j].dentlo_rasav = previous_j;
			}
			//sequential storing of pointer if same random location is generated again
			else
			{
				p = 0;
				while(1)
				{
					j = p;
					if(ran_test_array[previous_j].em_rasav == -1)
					{
						if(ran_test_array[j].dentlo_rasav == -1)
						{
							ran_test_array[previous_j].ptr = &ran_test_array[j];
							ran_test_array[previous_j].em_rasav = j;
							ran_test_array[j].dentlo_rasav = previous_j;
							break;
						}
					}
					p++;
				}
			}
		}
		else
		{
			k--;
		}
		previous_j = j;
		k++;
	}	

	for(l = 0; l < iterations; l++)			//accessing each block for iterations number of times
		for(j = 0; j < array_sizes; j++)				//writing data into one index of data array of array of elements randomly	
				ran_test_array[j].ptr->rant_data[0] = 1;	

	free(ran_test_array);		//free the array of blocks 	
	return 0;
}
