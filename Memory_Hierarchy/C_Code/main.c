#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define CASES 17         //considering the case of 17different array sizes

/* structure of cache line size for random memory access */
struct ran_test_data{
	int rant_data[12];		//data array
	int written_value;     //flag to keep track of written element
	int location_value;		//flag to keep track of location value
	struct ran_test_data *ptr;     //to hold address of random element
};

/* structure of cache line size for sequential memory access */
struct seq_test_data{
	int seqt_data[16];	   //data array
};

/*************** Main Program ****************/
int main(){
	/* to create different random values each time*/
	srand(time(NULL));    

	/* Array to hold different values of array sizes */
	int array_sizes[CASES] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};

	/* arrays to hold the calculated average memory access time */
	double seq_read_access_times[CASES] = {0};
	double seq_write_access_times[CASES] = {0};
	double ran_read_access_times[CASES] = {0};
	double ran_write_access_times[CASES] = {0};

	/* to hold the CPU clock cycles */
	long double clk_time;
	int i, j, k, l, p = 0, previous_j, iterations = 50000;

	/* to hold the read value from memory */
	register int read_val;

	/* to hold the start and end times of memory access */
	clock_t start, end;

	struct ran_test_data *ran_test_array;      //array of random data elements  
	struct seq_test_data *seq_test_array;		//array of sequential data elements

	/************************* For sequential operations **************************/
	for(i = 0; i < CASES; i++){	
		seq_test_array = malloc(sizeof(struct seq_test_data)*array_sizes[i]);    //to create array size number of elements

		/* to write some value before reading to the data array of each sequential element */
		for(l = 0;l < array_sizes[i]; l++){
			for(k = 0; k < 16; k++)
				seq_test_array[l].seqt_data[k] = 0;
		}
				
		/************ Sequential Read **************/
		start = clock();         //start time of read operation

		for(l = 0; l < iterations; l++)        //accessing each block for iterations number of times
			for(j = 0; j < array_sizes[i]; j++)			//accessing data from each element of array of elements sequentially
				read_val = seq_test_array[j].seqt_data[0];
		
		end = clock();     //end time of read operation

		/* random check operation using read value to bypass compiler optimization. The idea here is if the read_val is not used in the program
		after it is set, then compiler MAY optimize and may not execute the read loop.*/
		if(seq_test_array[j-1].seqt_data[k-1] == read_val - 1)
		{
			asm volatile("nop");
		}

		/* calculating the average access time for each memory reference */
		clk_time = (long double)(end - start);		
		clk_time = ((clk_time * 1000)/iterations)/array_sizes[i];		
		seq_read_access_times[i] = clk_time;
				
		/************* Sequential Write **************/
		start = clock();		//start time of write operation

		for(l = 0; l < iterations; l++)			//accessing each block for iterations number of times
			for(j = 0; j < array_sizes[i]; j++)		//writing data into one index of data array of array of elements sequentially
					seq_test_array[j].seqt_data[0] = 1;
		
		end = clock();	//end time of read operation

		/* calculating the average access time for each memory reference */
		clk_time = (long double)(end - start);		
		clk_time = ((clk_time * 1000)/iterations)/array_sizes[i];		
		seq_write_access_times[i] = clk_time;

		printf("sequential write and read time of %d array size = %f\t%f\n", array_sizes[i], seq_write_access_times[i], seq_read_access_times[i]);
		free(seq_test_array);		//free the array of blocks 
	}

	/********************* For random operations ***********************/
	for(i = 0; i < CASES; i++){
		ran_test_array = malloc(sizeof(struct ran_test_data)*array_sizes[i]);      //to create array size number of elements
		k = 0;

		/* initialization of flags of each element */
		while(k < array_sizes[i])
		{
			ran_test_array[k].location_value = -1;
			ran_test_array[k].written_value = -1;
			k++;
		}

		/******* Pointer Chasing *******/
		k = 0;	
		while(k < array_sizes[i])
		{
			j = rand()%array_sizes[i];    //generating random number
			
			/* storing the location of randomly generated index element into first element of array */
			if (ran_test_array[0].written_value == -1)
			{
				ran_test_array[0].ptr = &ran_test_array[j];
				previous_j = j; 
				ran_test_array[0].written_value = j;
				ran_test_array[j].location_value=0;
				k++;
				continue;	
			}

			if(ran_test_array[previous_j].written_value == -1)
			{
				if(ran_test_array[j].location_value == -1)
				{
					ran_test_array[previous_j].ptr = &ran_test_array[j];
					ran_test_array[previous_j].written_value = j;
					ran_test_array[j].location_value = previous_j;
				}

				//sequential storing of pointer if same random location is generated again
				else
				{
					p = 0;
					while(1)
					{
						j = p;
						if(ran_test_array[previous_j].written_value == -1)
						{
							if(ran_test_array[j].location_value == -1)
							{
								ran_test_array[previous_j].ptr = &ran_test_array[j];
								ran_test_array[previous_j].written_value = j;
								ran_test_array[j].location_value = previous_j;
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
		
			
		/********** Random Read ************/		
		start = clock();	//start time of read operation

		for(l = 0; l < iterations; l++)			//accessing each block for iterations number of times
			for(j = 0; j < array_sizes[i]; j++)			//accessing data from each element of array of elements sequentially	
					read_val = ran_test_array[j].ptr->rant_data[0];								
		
		end = clock();		//end time of read operation

		/* random check operation using read value to prevent compiler */
		if(ran_test_array[j-1].ptr->rant_data[0] == read_val-1)
		{
			asm volatile("nop");
		}

		/* calculating the average access time for each memory reference */
		clk_time = (long double)(end - start);		
		clk_time = ((clk_time * 1000)/iterations)/array_sizes[i];		
		ran_read_access_times[i] = clk_time;
		
		
		/************ Random Write ************/
		start = clock();	//start time of write operation

		for(l = 0; l < iterations; l++)		//accessing each block for iterations number of times
			for(j = 0; j < array_sizes[i]; j++)				//writing data into one index of data array of array of elements randomly	
					ran_test_array[j].ptr->rant_data[0] = 1;			
		
		end = clock();		//end time of write operation

		/* calculating the average access time for each memory reference */
		clk_time = (long double)(end - start);		
		clk_time = ((clk_time * 1000)/iterations)/array_sizes[i];		
		ran_write_access_times[i] = clk_time;

		printf("random write and read time of %d array size = %f\t%f\n", array_sizes[i], ran_write_access_times[i], ran_read_access_times[i]);		
		free(ran_test_array);		//free the array of blocks 	
	}	
	return 0;
}
