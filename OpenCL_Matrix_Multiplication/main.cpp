#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <string>

#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)
#define SIZE 1024 // column/row size.
#define TILE_SIZE 8 // column/row size in a tile. It can only be 1,2,4,8,16 because of Intel GPU hardware limitation.

#define max 20
#define min -20

// C version of matrix multiplcation. Use this function for result validation and execution time comaprison
void matrix_mul_sequence(int *A_mat,
	int *B_mat,
	int *C_mat)
{
	for (int j = 0; j<SIZE; j++) {
		for (int i = 0; i<SIZE; i++)
			for (int k = 0; k<SIZE; k++)
				C_mat[j*SIZE + i] += A_mat[j*SIZE + k] * B_mat[k*SIZE + i];
	}
}

int main(void)
{
	srand(time(NULL));
	// A, B are input matrix, C is the output matrix for OpenCL, C_seq is the output matrix for reference implementation.
	int *A = new int[SIZE*SIZE];
	int *B = new int[SIZE*SIZE];
	int *C = new int[SIZE*SIZE];
	int *C_seq = new int[SIZE*SIZE];

	//Initialize matrix
	for (int j = 0; j<SIZE; j++) {
		for (int i = 0; i<SIZE; i++) {
			// Initializing the matrix elements with random numbers between -20 and 20
			A[j*SIZE + i] = min + rand() % (max - min);
			B[j*SIZE + i] = min + rand() % (max - min);
			C[j*SIZE + i] = 0;
			C_seq[j*SIZE + i] = 0;
		}
	}

	std::chrono::high_resolution_clock::time_point t1, t2;
	t1 = std::chrono::high_resolution_clock::now();
	matrix_mul_sequence(A, B, C_seq);
	t2 = std::chrono::high_resolution_clock::now();
	std::cout << "Reference C matrix multiplication: "
		<< (float)(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
		<< " sec"
		<< std::endl;

	// Load the kernel source code into the array source_str
	FILE *fp;
	char *source_str;
	size_t source_size;

	fp = fopen("matrix_mul.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	//////////////////////////////////////// Tile Matrix Multiplication - GPU ///////////////////////////////////////////////

	// Get platform and device information
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);

	// Create an OpenCL context
	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

	// Create a command queue with the capability of performance profiling for target device
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);

	// Create memory buffers on the device for each matrix
	cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret);
	cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret);
	cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret);

	// Copy the matrix A, B and C to each device memory counterpart
	ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, SIZE*SIZE * sizeof(int), A, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, SIZE*SIZE * sizeof(int), B, 0, NULL, NULL);
	ret = clEnqueueWriteBuffer(command_queue, c_mem_obj, CL_TRUE, 0, SIZE*SIZE * sizeof(int), C, 0, NULL, NULL);

	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

	// Build and compile the OpenCL kernel program
	std::string build_option = "-DTILE_SIZE=" + std::to_string(TILE_SIZE);
	ret = clBuildProgram(program, 1, &device_id, build_option.c_str(), NULL, NULL);
	if (ret == CL_BUILD_PROGRAM_FAILURE) { // If compile failed, print error message
										   // Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = (char *)malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		printf("%s\n", log);
	}

	// Create the OpenCL kernel
	cl_kernel kernel;
	kernel = clCreateKernel(program, "matrix_mul_tile", &ret);

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);

	int dimention = 2; // In this example, We will use 2 dimention index
	size_t global_item_size[] = { SIZE, SIZE, 1 };
	size_t local_item_size[] = { TILE_SIZE, TILE_SIZE, 1 };

	cl_event perf_event;
	cl_ulong start, end;

	// Execute the OpenCL kernel
	ret = clEnqueueNDRangeKernel(command_queue, kernel, dimention, NULL, global_item_size, local_item_size, 0, NULL, &perf_event);
	// Capture performance event from target device. In this case the event is to retrive execution time.
	ret = clWaitForEvents(1, &perf_event);
	ret = clGetEventProfilingInfo(perf_event, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
	ret = clGetEventProfilingInfo(perf_event, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);
	std::cout << "OpenCL tile matrix multiplication on GPU: " << (float)(end - start) / 1000000000 << " sec" << std::endl;

	// Read the memory buffer C on the device to the local variable C
	ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, SIZE*SIZE * sizeof(int), C, 0, NULL, NULL);

	// Make sure all the command in the command queue has been executed
	ret = clFinish(command_queue);

	//////////////////////////////////////// Normal Matrix Multiplication - GPU ///////////////////////////////////////////////

	// Get platform and device information
	cl_platform_id platform_id1 = NULL;
	cl_device_id device_id1 = NULL;
	cl_uint ret_num_devices1;
	cl_uint ret_num_platforms1;
	cl_int ret1 = clGetPlatformIDs(1, &platform_id1, &ret_num_platforms1);
	ret1 = clGetDeviceIDs(platform_id1, CL_DEVICE_TYPE_GPU, 1, &device_id1, &ret_num_devices1);

	// Create an OpenCL context
	cl_context context1 = clCreateContext(NULL, 1, &device_id1, NULL, NULL, &ret1);

	// Create a command queue with the capability of performance profiling for target device
	cl_command_queue command_queue1 = clCreateCommandQueue(context1, device_id1, CL_QUEUE_PROFILING_ENABLE, &ret1);

	// Create memory buffers on the device for each matrix
	cl_mem a_mem_obj1 = clCreateBuffer(context1, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret1);
	cl_mem b_mem_obj1 = clCreateBuffer(context1, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret1);
	cl_mem c_mem_obj1 = clCreateBuffer(context1, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret1);

	// Copy the matrix A, B and C to each device memory counterpart
	ret1 = clEnqueueWriteBuffer(command_queue1, a_mem_obj1, CL_TRUE, 0, SIZE*SIZE * sizeof(int), A, 0, NULL, NULL);
	ret1 = clEnqueueWriteBuffer(command_queue1, b_mem_obj1, CL_TRUE, 0, SIZE*SIZE * sizeof(int), B, 0, NULL, NULL);
	ret1 = clEnqueueWriteBuffer(command_queue1, c_mem_obj1, CL_TRUE, 0, SIZE*SIZE * sizeof(int), C, 0, NULL, NULL);

	// Create a program from the kernel source
	cl_program program1 = clCreateProgramWithSource(context1, 1, (const char **)&source_str, (const size_t *)&source_size, &ret1);

	// Build and compile the OpenCL kernel program
	std::string build_option1 = "-DTILE_SIZE=" + std::to_string(TILE_SIZE);
	ret1 = clBuildProgram(program1, 1, &device_id1, build_option1.c_str(), NULL, NULL);
	if (ret1 == CL_BUILD_PROGRAM_FAILURE) { // If compile failed, print error message
											// Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program1, device_id1, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = (char *)malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(program1, device_id1, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		printf("%s\n", log);
	}

	// Create the OpenCL kernel
	cl_kernel kernel1;
	kernel1 = clCreateKernel(program1, "matrix_mul", &ret1);

	// Set the arguments of the kernel
	ret1 = clSetKernelArg(kernel1, 0, sizeof(cl_mem), (void *)&a_mem_obj1);
	ret1 = clSetKernelArg(kernel1, 1, sizeof(cl_mem), (void *)&b_mem_obj1);
	ret1 = clSetKernelArg(kernel1, 2, sizeof(cl_mem), (void *)&c_mem_obj1);

	int dimention1 = 2; // In this example, We will use 2 dimention index
	size_t global_item_size1[] = { SIZE, SIZE, 1 };
	size_t local_item_size1[] = { TILE_SIZE, TILE_SIZE, 1 };

	cl_event perf_event1;
	cl_ulong start1, end1;

	// Execute the OpenCL kernel
	ret1 = clEnqueueNDRangeKernel(command_queue1, kernel1, dimention1, NULL, global_item_size1, local_item_size1, 0, NULL, &perf_event1);
	// Capture performance event from target device. In this case the event is to retrive execution time.
	ret1 = clWaitForEvents(1, &perf_event1);
	ret1 = clGetEventProfilingInfo(perf_event1, CL_PROFILING_COMMAND_START, sizeof(start1), &start1, NULL);
	ret1 = clGetEventProfilingInfo(perf_event1, CL_PROFILING_COMMAND_END, sizeof(end1), &end1, NULL);
	std::cout << "OpenCL normal matrix multiplication on GPU: " << (float)(end1 - start1) / 1000000000 << " sec" << std::endl;

	// Read the memory buffer C on the device to the local variable C
	ret1 = clEnqueueReadBuffer(command_queue1, c_mem_obj1, CL_TRUE, 0, SIZE*SIZE * sizeof(int), C, 0, NULL, NULL);

	// Make sure all the command in the command queue has been executed
	ret1 = clFinish(command_queue1);

	//////////////////////////////////////// Normal Matrix Multiplication - CPU ///////////////////////////////////////////////

	// Get platform and device information
	cl_platform_id platform_id_cpu_1 = NULL;
	cl_device_id device_id_cpu_1 = NULL;
	cl_uint ret_num_devices_cpu_1;
	cl_uint ret_num_platforms_cpu_1;
	cl_int ret_cpu_1 = clGetPlatformIDs(1, &platform_id_cpu_1, &ret_num_platforms_cpu_1);
	ret_cpu_1 = clGetDeviceIDs(platform_id_cpu_1, CL_DEVICE_TYPE_CPU, 1, &device_id_cpu_1, &ret_num_devices_cpu_1);

	// Create an OpenCL context
	cl_context context_cpu_1 = clCreateContext(NULL, 1, &device_id_cpu_1, NULL, NULL, &ret_cpu_1);

	// Create a command queue with the capability of performance profiling for target device
	cl_command_queue command_queue_cpu_1 = clCreateCommandQueue(context_cpu_1, device_id_cpu_1, CL_QUEUE_PROFILING_ENABLE, &ret_cpu_1);

	// Create memory buffers on the device for each matrix
	cl_mem a_mem_obj_cpu_1 = clCreateBuffer(context_cpu_1, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret_cpu_1);
	cl_mem b_mem_obj_cpu_1 = clCreateBuffer(context_cpu_1, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret_cpu_1);
	cl_mem c_mem_obj_cpu_1 = clCreateBuffer(context_cpu_1, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret_cpu_1);

	// Copy the matrix A, B and C to each device memory counterpart
	ret_cpu_1 = clEnqueueWriteBuffer(command_queue_cpu_1, a_mem_obj_cpu_1, CL_TRUE, 0, SIZE*SIZE * sizeof(int), A, 0, NULL, NULL);
	ret_cpu_1 = clEnqueueWriteBuffer(command_queue_cpu_1, b_mem_obj_cpu_1, CL_TRUE, 0, SIZE*SIZE * sizeof(int), B, 0, NULL, NULL);
	ret_cpu_1 = clEnqueueWriteBuffer(command_queue_cpu_1, c_mem_obj_cpu_1, CL_TRUE, 0, SIZE*SIZE * sizeof(int), C, 0, NULL, NULL);

	// Create a program from the kernel source
	cl_program program_cpu_1 = clCreateProgramWithSource(context_cpu_1, 1, (const char **)&source_str, (const size_t *)&source_size, &ret_cpu_1);

	// Build and compile the OpenCL kernel program
	std::string build_option_cpu_1 = "-DTILE_SIZE=" + std::to_string(TILE_SIZE);
	ret_cpu_1 = clBuildProgram(program_cpu_1, 1, &device_id_cpu_1, build_option_cpu_1.c_str(), NULL, NULL);
	if (ret_cpu_1 == CL_BUILD_PROGRAM_FAILURE) { // If compile failed, print error message
												 // Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program_cpu_1, device_id_cpu_1, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = (char *)malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(program_cpu_1, device_id_cpu_1, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		printf("%s\n", log);
	}

	// Create the OpenCL kernel
	cl_kernel kernel_cpu_1;
	kernel_cpu_1 = clCreateKernel(program_cpu_1, "matrix_mul", &ret_cpu_1);

	// Set the arguments of the kernel
	ret_cpu_1 = clSetKernelArg(kernel_cpu_1, 0, sizeof(cl_mem), (void *)&a_mem_obj_cpu_1);
	ret_cpu_1 = clSetKernelArg(kernel_cpu_1, 1, sizeof(cl_mem), (void *)&b_mem_obj_cpu_1);
	ret_cpu_1 = clSetKernelArg(kernel_cpu_1, 2, sizeof(cl_mem), (void *)&c_mem_obj_cpu_1);

	int dimention_cpu_1 = 2; // In this example, We will use 2 dimention index
	size_t global_item_size_cpu_1[] = { SIZE, SIZE, 1 };
	size_t local_item_size_cpu_1[] = { TILE_SIZE, TILE_SIZE, 1 };

	cl_event perf_event_cpu_1;
	cl_ulong start_cpu_1, end_cpu_1;

	// Execute the OpenCL kernel
	ret_cpu_1 = clEnqueueNDRangeKernel(command_queue_cpu_1, kernel_cpu_1, dimention_cpu_1, NULL, global_item_size_cpu_1, local_item_size_cpu_1, 0, NULL, &perf_event_cpu_1);
	// Capture performance event from target device. In this case the event is to retrive execution time.
	ret_cpu_1 = clWaitForEvents(1, &perf_event_cpu_1);
	ret_cpu_1 = clGetEventProfilingInfo(perf_event_cpu_1, CL_PROFILING_COMMAND_START, sizeof(start_cpu_1), &start_cpu_1, NULL);
	ret_cpu_1 = clGetEventProfilingInfo(perf_event_cpu_1, CL_PROFILING_COMMAND_END, sizeof(end_cpu_1), &end_cpu_1, NULL);
	std::cout << "OpenCL normal matrix multiplication on CPU: " << (float)(end_cpu_1 - start_cpu_1) / 1000000000 << " sec" << std::endl;

	// Read the memory buffer C on the device to the local variable C
	ret_cpu_1 = clEnqueueReadBuffer(command_queue_cpu_1, c_mem_obj_cpu_1, CL_TRUE, 0, SIZE*SIZE * sizeof(int), C, 0, NULL, NULL);

	// Make sure all the command in the command queue has been executed
	ret_cpu_1 = clFinish(command_queue_cpu_1);

	//////////////////////////////////////// Tile Matrix Multiplication - CPU ///////////////////////////////////////////////

	// Get platform and device information
	cl_platform_id platform_id_cpu_2 = NULL;
	cl_device_id device_id_cpu_2 = NULL;
	cl_uint ret_num_devices_cpu_2;
	cl_uint ret_num_platforms_cpu_2;
	cl_int ret_cpu_2 = clGetPlatformIDs(1, &platform_id_cpu_2, &ret_num_platforms_cpu_2);
	ret_cpu_2 = clGetDeviceIDs(platform_id_cpu_2, CL_DEVICE_TYPE_CPU, 1, &device_id_cpu_2, &ret_num_devices_cpu_2);

	// Create an OpenCL context
	cl_context context_cpu_2 = clCreateContext(NULL, 1, &device_id_cpu_2, NULL, NULL, &ret_cpu_2);

	// Create a command queue with the capability of performance profiling for target device
	cl_command_queue command_queue_cpu_2 = clCreateCommandQueue(context_cpu_2, device_id_cpu_2, CL_QUEUE_PROFILING_ENABLE, &ret_cpu_2);

	// Create memory buffers on the device for each matrix
	cl_mem a_mem_obj_cpu_2 = clCreateBuffer(context_cpu_2, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret_cpu_2);
	cl_mem b_mem_obj_cpu_2 = clCreateBuffer(context_cpu_2, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret_cpu_2);
	cl_mem c_mem_obj_cpu_2 = clCreateBuffer(context_cpu_2, CL_MEM_READ_WRITE, SIZE*SIZE * sizeof(int), NULL, &ret_cpu_2);

	// Copy the matrix A, B and C to each device memory counterpart
	ret_cpu_2 = clEnqueueWriteBuffer(command_queue_cpu_2, a_mem_obj_cpu_2, CL_TRUE, 0, SIZE*SIZE * sizeof(int), A, 0, NULL, NULL);
	ret_cpu_2 = clEnqueueWriteBuffer(command_queue_cpu_2, b_mem_obj_cpu_2, CL_TRUE, 0, SIZE*SIZE * sizeof(int), B, 0, NULL, NULL);
	ret_cpu_2 = clEnqueueWriteBuffer(command_queue_cpu_2, c_mem_obj_cpu_2, CL_TRUE, 0, SIZE*SIZE * sizeof(int), C, 0, NULL, NULL);

	// Create a program from the kernel source
	cl_program program_cpu_2 = clCreateProgramWithSource(context_cpu_2, 1, (const char **)&source_str, (const size_t *)&source_size, &ret_cpu_2);

	// Build and compile the OpenCL kernel program
	std::string build_option_cpu_2 = "-DTILE_SIZE=" + std::to_string(TILE_SIZE);
	ret_cpu_2 = clBuildProgram(program_cpu_2, 1, &device_id_cpu_2, build_option_cpu_2.c_str(), NULL, NULL);
	if (ret_cpu_2 == CL_BUILD_PROGRAM_FAILURE) { // If compile failed, print error message
												 // Determine the size of the log
		size_t log_size;
		clGetProgramBuildInfo(program_cpu_2, device_id_cpu_2, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = (char *)malloc(log_size);

		// Get the log
		clGetProgramBuildInfo(program_cpu_2, device_id_cpu_2, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		// Print the log
		printf("%s\n", log);
	}

	// Create the OpenCL kernel
	cl_kernel kernel_cpu_2;
	kernel_cpu_2 = clCreateKernel(program_cpu_2, "matrix_mul_tile", &ret_cpu_2);

	// Set the arguments of the kernel
	ret_cpu_2 = clSetKernelArg(kernel_cpu_2, 0, sizeof(cl_mem), (void *)&a_mem_obj_cpu_2);
	ret_cpu_2 = clSetKernelArg(kernel_cpu_2, 1, sizeof(cl_mem), (void *)&b_mem_obj_cpu_2);
	ret_cpu_2 = clSetKernelArg(kernel_cpu_2, 2, sizeof(cl_mem), (void *)&c_mem_obj_cpu_2);

	int dimention_cpu_2 = 2; // In this example, We will use 2 dimention index
	size_t global_item_size_cpu_2[] = { SIZE, SIZE, 1 };
	size_t local_item_size_cpu_2[] = { TILE_SIZE, TILE_SIZE, 1 };

	cl_event perf_event_cpu_2;
	cl_ulong start_cpu_2, end_cpu_2;

	// Execute the OpenCL kernel
	ret_cpu_2 = clEnqueueNDRangeKernel(command_queue_cpu_2, kernel_cpu_2, dimention_cpu_2, NULL, global_item_size_cpu_2, local_item_size_cpu_2, 0, NULL, &perf_event_cpu_2);
	// Capture performance event from target device. In this case the event is to retrive execution time.
	ret_cpu_2 = clWaitForEvents(1, &perf_event_cpu_2);
	ret_cpu_2 = clGetEventProfilingInfo(perf_event_cpu_2, CL_PROFILING_COMMAND_START, sizeof(start_cpu_2), &start_cpu_2, NULL);
	ret_cpu_2 = clGetEventProfilingInfo(perf_event_cpu_2, CL_PROFILING_COMMAND_END, sizeof(end_cpu_2), &end_cpu_2, NULL);
	std::cout << "OpenCL tile matrix multiplication on CPU: " << (float)(end_cpu_2 - start_cpu_2) / 1000000000 << " sec" << std::endl;

	// Read the memory buffer C on the device to the local variable C
	ret_cpu_2 = clEnqueueReadBuffer(command_queue_cpu_2, c_mem_obj_cpu_2, CL_TRUE, 0, SIZE*SIZE * sizeof(int), C, 0, NULL, NULL);

	// Make sure all the command in the command queue has been executed
	ret_cpu_2 = clFinish(command_queue_cpu_2);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool validate = true;
	for (int j = 0; j<SIZE; j++) {
		for (int i = 0; i<SIZE; i++) {
			if (C[j*SIZE + i] != C_seq[j*SIZE + i])
				validate = false;
		}
	}

	if (validate == false)
		std::cout << "The results are mismatched !!" << std::endl;

	// Clean up
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(a_mem_obj);
	ret = clReleaseMemObject(b_mem_obj);
	ret = clReleaseMemObject(c_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	ret = clReleaseKernel(kernel1);
	ret = clReleaseProgram(program1);
	ret = clReleaseMemObject(a_mem_obj1);
	ret = clReleaseMemObject(b_mem_obj1);
	ret = clReleaseMemObject(c_mem_obj1);
	ret = clReleaseCommandQueue(command_queue1);
	ret = clReleaseContext(context1);

	ret = clReleaseKernel(kernel_cpu_1);
	ret = clReleaseProgram(program_cpu_1);
	ret = clReleaseMemObject(a_mem_obj_cpu_1);
	ret = clReleaseMemObject(b_mem_obj_cpu_1);
	ret = clReleaseMemObject(c_mem_obj_cpu_1);
	ret = clReleaseCommandQueue(command_queue_cpu_1);
	ret = clReleaseContext(context_cpu_1);

	ret = clReleaseKernel(kernel_cpu_2);
	ret = clReleaseProgram(program_cpu_2);
	ret = clReleaseMemObject(a_mem_obj_cpu_2);
	ret = clReleaseMemObject(b_mem_obj_cpu_2);
	ret = clReleaseMemObject(c_mem_obj_cpu_2);
	ret = clReleaseCommandQueue(command_queue_cpu_2);
	ret = clReleaseContext(context_cpu_2);

	std::cout << "Press Enter to finish..." << std::endl;
	getchar();
	return 0;
}