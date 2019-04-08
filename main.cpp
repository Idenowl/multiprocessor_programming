#define __CL_ENABLE_EXCEPTIONS
#define _CRT_SECURE_NO_DEPRECATE
#include <CL/opencl.h>
#include <CL/cl.h>
#include<iostream>
#include<string>
#include<vector>
#include "lodepng.h"
#include <cmath>
#include <windows.h>
#include <omp.h>

#define DATA_SIZE (2048)


int main() {
	//////////////////////////////////////////////////////////////////////////DECODING IMAGE ////////////////////////////////////////////////////
	//Decode the image, source lodepng.h example

	const char* filename = "img/im0.png";
	unsigned char * image;

	unsigned  width, height;

	unsigned error = lodepng_decode32_file(&image, &width, &height, filename);
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	//Decode image1 
	const char* filename1 = "img/im1.png";

	unsigned char * image1;

	unsigned error2 = lodepng_decode32_file(&image1, &width, &height, filename1);
	if (error) std::cout << "decoder error " << error2 << ": " << lodepng_error_text(error2) << std::endl;

	////////////////////////////////////////////////////////////////////////////////GET PLATEFORM & DEVICES/////////////////////////////////////////////

	int i, j;
	char* value;
	size_t valueSize;
	cl_uint platformCount;
	cl_platform_id* platforms;
	cl_uint deviceCount;
	cl_device_id* devices;
	cl_ulong localmemsize;
	cl_device_local_mem_type localmemtype;
	cl_uint maxComputeUnits;
	cl_uint maxClockfrequency;
	cl_ulong maxConstantbuffersize;
	size_t maxWorkgroupsize;
	cl_uint maxDimensions;
	//size_t*  maxWorkitemsize;


	//get platform 

	clGetPlatformIDs(0, NULL, &platformCount);
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * platformCount);
	clGetPlatformIDs(platformCount, platforms, NULL);
	//get devices 
	clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &deviceCount);
	devices = (cl_device_id*)malloc(sizeof(cl_device_id) * deviceCount);
	clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, deviceCount, devices, NULL);
	printf("Device count %d\n", deviceCount);
	char device_string[1024];


	//////////////////////////////////////////////////////////////////////////GET DEVICE INFO//////////////////////////////////
	clGetDeviceInfo(devices[0], CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
	printf("  CL_DEVICE_NAME: \t\t\t%s\n", device_string);

	clGetDeviceInfo(devices[0], CL_DEVICE_LOCAL_MEM_TYPE, sizeof(localmemtype), &localmemtype, NULL);
	printf("  CL_DEVICE_LOCAL_MEM_TYPE: \t\t\t%u\n", localmemtype);

	clGetDeviceInfo(devices[0], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(localmemsize), &localmemsize, NULL);
	printf("  CL_DEVICE_LOCAL_MEM_SIZE: \t\t\t%d\n", localmemsize);

	clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, NULL);
	printf("  CL_DEVICE_MAX_COMPUTE_UNITS: \t\t\t%d\n", maxComputeUnits);

	clGetDeviceInfo(devices[0], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(maxClockfrequency), &maxClockfrequency, NULL);
	printf(" CL_DEVICE_MAX_CLOCK_FREQUENCY: \t\t\t%d\n", maxClockfrequency);

	clGetDeviceInfo(devices[0], CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(maxConstantbuffersize), &maxConstantbuffersize, NULL);
	printf(" CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE: \t\t\t%d\n", maxConstantbuffersize);

	clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkgroupsize), &maxWorkgroupsize, NULL);
	printf(" CL_DEVICE_MAX_WORK_GROUP_SIZE: \t\t\t%d\n", maxWorkgroupsize);

	clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(maxDimensions), &maxDimensions, NULL);
	printf(" CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS: \t\t\t%d\n", maxDimensions);

	//clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, maxDimensions * sizeof(size_t), &maxWorkitemsize, NULL);
	//printf(" CL_DEVICE_MAX_WORK_ITEM_SIZES: \t\t\t%d\n", maxWorkitemsize);

	////////////////////////////////////////////////////////CREATE CONTEXT AND MEMORY OBJECTS //////////////////////////////////////////////////
	//Create Context
	cl_int err_ret;
	cl_context context = clCreateContext(NULL, deviceCount, &devices[0], NULL, NULL, &err_ret);


	//Memory object Image-object
	int divide = 4;
	size_t image_width = width;
	size_t image_height = height;

	const cl_image_format image_format = { CL_RGBA, CL_UNSIGNED_INT8 };
	const cl_image_format image_format_output = { CL_RGBA, CL_UNSIGNED_INT8 };

	cl_int err;
	cl_mem img1 = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &image_format, image_width, image_height, 0, image, &err);
	if (err != CL_SUCCESS) { printf("ERROR Image1 %d \n", err); }
	cl_mem img1_output = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &image_format_output, width, height, 0, NULL, &err);
	cl_mem img1_output2 = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &image_format_output, width, height, 0, NULL, &err);

	//cl_mem img2 = clCreateImage2D(context, CL_MEM_READ_ONLY, &image_format, image_width, image_height, 0, image1, &err);
	//cl_mem img2_output = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &image_format, image_width, image_height, 0, NULL, &err);
	if (err != CL_SUCCESS) { printf("ERROR Image2 %d \n", err); }

	//////////////////////////////////////////////////COMMAND QUEUE//////////////////////////////////////////////////////////////////////
	//Create command queue
	cl_command_queue commands = clCreateCommandQueue(context, devices[0], 0, &err);
	

	/////////////////////////////////////////////////////////////////////LOAD KERNEL///////////////////////////////////////////////////////////////

	//Load the kernels 
	FILE *fp;
	char *resize_str;
	size_t resize_size;

	fp = fopen("resize.cl", "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	resize_str = (char*)malloc(DATA_SIZE);
	resize_size = fread(resize_str, 1, DATA_SIZE, fp);
	fclose(fp);

	FILE *fp2;
	char *imgtogrey_str;
	size_t imgtogrey_size;

	fp2 = fopen("imgtogrey.cl", "r");
	if (!fp2) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	imgtogrey_str = (char*)malloc(DATA_SIZE);
	imgtogrey_size = fread(imgtogrey_str, 1, DATA_SIZE, fp2);
	fclose(fp2);


	/////////////////////////////////////////////////CREATE AND BUILD KERNEL/////////////////////////////////////////////////////////////////////////////////////////
	//create and build the program resize
	//const char** list_kernel = (const char**)malloc(2 * DATA_SIZE);
	//int * list_size = (int*)malloc(imgtogrey_size + resize_size*sizeof(size_t));
	//list_kernel[0] = resize_str; 
	//list_kernel[1] = imgtogrey_str; 
	//list_size[0] = resize_size; 
	//list_size[1] = imgtogrey_size;

	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&resize_str, (const size_t *)&resize_size, &err);
	//cl_program program = clCreateProgramWithSource(context, 1, list_kernel, (const size_t *)list_size, &err);
	if (err != CL_SUCCESS) { printf("create program error %d", err); }
	err = clBuildProgram(program, 1, &devices[0], NULL, NULL, NULL);
	if (err!=CL_SUCCESS) { 
		printf("Program resize ERROR %d \n",err);
		size_t log_size;
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = (char *)malloc(log_size);
		clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		printf("%s\n", log);
	}
	cl_kernel resize_kernel = clCreateKernel(program, "change_size", &err);
	

	
	//grey 
	//cl_program grey = clCreateProgramWithSource(context, 1, (const char **)&imgtogrey_str, (const size_t *)&imgtogrey_size, &err);
	//err = clBuildProgram(program, 1, &devices[0], NULL, NULL, NULL);
	//if (err != CL_SUCCESS) {
	//	printf("Program Grey ERROR %d \n", err);
	//	size_t log_size;
	//	clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	//	char *log = (char *)malloc(log_size);
	//	clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		
	//	printf("%s\n", log);
	//}
	
	//cl_kernel grey_kernel = clCreateKernel(program, "to_grey", &err);
	//if (err != CL_SUCCESS) { printf("Kernel ERROR %d \n", err); }
	///////////////////////////////////////////////////////////////////////SET ARGUMENT ON KERNEL///////////////////////////////////////////////////////////////////
	//resize set argument 
	err = clSetKernelArg(resize_kernel, 0, sizeof(cl_mem), &img1);
	err |= clSetKernelArg(resize_kernel, 1, sizeof(cl_mem), &img1_output);
	if (err) { printf("Argument ERROR %d \n", err); }
	
	//Grey set argument 
	//err = clSetKernelArg(grey_kernel, 0, sizeof(cl_mem), &img1);
	//err |= clSetKernelArg(grey_kernel, 1, sizeof(cl_mem), &img1_output);
	//if (err) { printf("Argument ERROR grey %d \n", err); }


	/////////////////////////////////////////////////////////////////////EXECUTION////////////////////////////////////////////////////////////////////////////////////
	//execute kernel 
	size_t global_work_size[2] = { width,height}; 
	size_t local_work_size[2] = { 1,1 };
	
	err = clEnqueueNDRangeKernel(commands, resize_kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	
	if (err) { printf("kernel execution ERROR %d \n",err); }
	//err = clEnqueueNDRangeKernel(commands, grey_kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);

	////////////////////////////////////////////////////////////////////GET OUTPUT////////////////////////////////////////////////////////////////////////////////
	//get data calculated in device memory 
	size_t origin[3] = { 0, 0, 0 };
	size_t region[3] = { width/4, height/4, 1 };
	unsigned char* image_output =(unsigned char*) malloc(sizeof(unsigned char) * width * height*4 );
	if (image_output == NULL) { printf("ALLOCATION PROBLEM"); }
	err=clEnqueueReadImage(commands, img1_output, CL_TRUE, origin, region, 0, 0,image_output,0,0,0);
	if (err) { printf("get data error %d \n", err); }
	
	//validation
	const char* filenameori = "IMG_CL/origine.png";

	const char* filename2 = "IMG_CL/test.png";
	LodePNGColorType colortype = LCT_RGBA;
	unsigned bitdepth = 8;
	err=lodepng_encode_file(filename2, image_output, width/4, height/4, colortype, bitdepth);
	err = lodepng_encode_file(filenameori, image, width, height, colortype, bitdepth);

}