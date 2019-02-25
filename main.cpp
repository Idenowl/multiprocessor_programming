#
#include<iostream>
#include<iostream>
#include<string>
#include<vector>
#include "lodepng.h"
#include <cmath>

unsigned char* change_size_RGBA(unsigned char* imagein,int width,int height) {
	// imagein(unsigned char*) image to reduct; RGBA
	// width, initial width of the image to reduct 
	// height initial height of the image to reduc 
	
	unsigned char* imgreduct = (unsigned char*)malloc(width*height * 4 *sizeof(char));
	int k = 0; 
	for (int h = 0; h < height; h = h + 4) {
		for (int w= 0; w< width ; w = w + 4) {
			//4 channels, RGBA make one pixel 
			// In the image(char*) we have the several lines one after the other
			//4*actual number of the line*width of one line+ 4* actual position in the line +channel (R=0 G B or A )
			imgreduct [k] = imagein [4* width* h + 4* w ]; //R
			imgreduct[k + 1] = imagein[4*width*h +4* w + 1]; //G
			imgreduct[k + 2] = imagein[4*width*h + 4*w + 2];//B
			imgreduct[k + 3] = imagein[4*width*h +4* w + 3];//A
			k = k + 4;
		}
	
	}

	return imgreduct;
}
unsigned char* RGB_to_grey(unsigned char* imagein, int width, int height) {
	unsigned char* imggrey = (unsigned char*)malloc(width*height*3*sizeof(char));
	int k = 0;
	for (int h = 0; h < height; h = h +1) {
		for (int w = 0; w < width; w = w + 1) {
			
			imggrey[k] = 0.2126*imagein[3 * width* h + 3 * w] + 0.7152*imagein[3 * width* h + 3 * w + 1] + 0.0722*imagein[3 * width* h + 3 * w + 2];
			k = k + 1;
			
		}
	}
	return imggrey;
}

unsigned char* change_size_RGB(unsigned char* imagein, int width, int height) {
	// imagein(unsigned char*) image to reduct; RGBA
	// width, initial width of the image to reduct 
	// height initial height of the image to reduc 

	unsigned char* imgreduct = (unsigned char*)malloc(width*height * 3 * sizeof(char));
	int k = 0;
	for (int h = 0; h < height; h = h + 4) {
		for (int w = 0; w < width; w = w + 4) {
			//4 channels, RGBA make one pixel 
			// In the image(char*) we have the several lines one after the other
			//4*actual number of the line*width of one line+ 4* actual position in the line +channel (R=0 G B or A )
			imgreduct[k] = imagein[4 * width* h + 4 * w]; //R
			imgreduct[k + 1] = imagein[4 * width*h + 4 * w + 1]; //G
			imgreduct[k + 2] = imagein[4 * width*h + 4 * w + 2];//B
			k = k + 3;
		}

	}

	return imgreduct;
}
unsigned char* ZNCC(unsigned char* imageR,unsigned char* imageL,int width,int height,int disp_max,int win_size) {
	//unsigned char* meanR =(unsigned char*)malloc(width*height); 
	//unsigned char* meanL =(unsigned char*)malloc(width*height);
	unsigned char* disparity_image = (unsigned char*)malloc(width*height);
	
	
	int win_half_value = 0.5 * (win_size - 1); 
	
	for (int j = win_half_value; j < (height- win_half_value); j++) {
		
		for (int i =win_half_value; i < (width - win_half_value);i++) {
			float max_sum = 0; // best sum
			int best_d = 0; //best disparity value
			for (int d = 0; d < disp_max; d++) {
				int meanR = 0; //mean image right
				int meanL = 0; //mean image left
				 
				for (int win_y = -win_half_value; win_y <= win_half_value; win_y++) {
					for (int win_x= -win_half_value; win_x <= win_half_value; win_x++) {
						
						meanR+=int(imageR[width*(j+win_y )+ i+win_x - d]);
						meanL+=int(imageL[width*(j + win_y) + i+win_x]);
						
					}
				}
				meanR = meanR / (win_size*win_size);
				meanL = meanL / (win_size*win_size);
				
				int sum_mean = 0;
				float sum_varR = 0; 
				float sum_varL = 0;
				for (int win_y = -win_half_value; win_y <= win_half_value; win_y++) {
					for (int win_x = -win_half_value; win_x <= win_half_value; win_x++) {
						sum_mean += (imageL[width*(j+win_y) + i+win_x] - meanL)*(imageR[width*(j+win_y) + i+win_x - d] - meanR);
						sum_varL += (imageL[width*(j + win_y) + i + win_x] - meanL)*(imageL[width*(j + win_y) + i + win_x] - meanL);
						sum_varR += (imageR[width*(j + win_y) + i + win_x - d] - meanR)*(imageR[width*(j + win_y) + i + win_x - d] - meanR);
						
					}
				}
				float ZNCC_value = sum_mean / (sqrt(sum_varR)*sqrt(sum_varL));
				
					if (ZNCC_value > max_sum){
						max_sum = ZNCC_value; 
						best_d = d; 
					}
			}
			disparity_image[width*j + i] =int(best_d*255/(disp_max));
			
		}
	}
	return disparity_image;
}
unsigned char* ZNCC2(unsigned char* imageR, unsigned char* imageL, int width, int height, int disp_max, int win_size) {
	//unsigned char* meanR =(unsigned char*)malloc(width*height); 
	//unsigned char* meanL =(unsigned char*)malloc(width*height);
	unsigned char* disparity_image = (unsigned char*)malloc(width*height);


	int win_half_value = 0.5 * (win_size - 1);

	for (int j = win_half_value; j < (height - win_half_value); j++) {

		for (int i = win_half_value; i < (width - win_half_value);i++) {
			float max_sum = 0; // best sum
			int best_d = 0; //best disparity value
			for (int d = 0; d < disp_max; d++) {
				int meanR = 0; //mean image right
				int meanL = 0; //mean image left

				for (int win_y = -win_half_value; win_y <= win_half_value; win_y++) {
					for (int win_x = -win_half_value; win_x <= win_half_value; win_x++) {

						meanR += int(imageR[width*(j + win_y) + i + win_x]);
						meanL += int(imageL[width*(j + win_y) + i + win_x + d]);

					}
				}
				meanR = meanR / (win_size*win_size);
				meanL = meanL / (win_size*win_size);

				int sum_mean = 0;
				float sum_varR = 0;
				float sum_varL = 0;
				for (int win_y = -win_half_value; win_y <= win_half_value; win_y++) {
					for (int win_x = -win_half_value; win_x <= win_half_value; win_x++) {
						sum_mean += (imageL[width*(j + win_y) + i + win_x+ d] - meanL)*(imageR[width*(j + win_y) + i + win_x] - meanR);
						sum_varL += (imageL[width*(j + win_y) + i + win_x + d] - meanL)*(imageL[width*(j + win_y) + i + win_x+ d] - meanL);
						sum_varR += (imageR[width*(j + win_y) + i + win_x] - meanR)*(imageR[width*(j + win_y) + i + win_x] - meanR);

					}
				}
				float ZNCC_value = sum_mean / (sqrt(sum_varR)*sqrt(sum_varL));

				if (ZNCC_value > max_sum) {
					max_sum = ZNCC_value;
					best_d = d;
				}
			}
			disparity_image[width*j + i] = int(best_d * 255 / (disp_max));

		}
	}
	return disparity_image;
}
unsigned char* Cross_checking(unsigned char* disparityR, unsigned char* disparityL, int width, int height, int threshold) {
	unsigned char* output = (unsigned char*)malloc(width*height);
	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			int difference = std::abs(disparityR[h*width + w] - disparityL[h*width + w]);
			if (difference > threshold) {
				output[h*width + w] = 0;
			}
			else {
				output[h*width + w] = disparityL[h*width + w];
			}
		}

	}
	return output;
}
int main() {
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

	//Processing 
	unsigned char* imgreduct = change_size_RGB(image, width, height);
	unsigned char* imggrey = RGB_to_grey(imgreduct, width / 4, height / 4);

	unsigned char* imgreduct1 = change_size_RGB(image1, width, height);
	unsigned char* imggrey1 = RGB_to_grey(imgreduct1, width / 4, height / 4);

	//ZNCC algorithm 
	unsigned char* disparity_image = ZNCC(imggrey1, imggrey, width / 4, height / 4, 65, 9);
	unsigned char* disparity_image_right = ZNCC2(imggrey1, imggrey, width / 4, height / 4, 65, 9);

	//Cross Checking 
	unsigned char* crosscheck = Cross_checking(disparity_image_right, disparity_image, width / 4, height / 4, 8);
	


	////////////////////////////////////////ENCODING PART /////////////////////////////////////////////////////////
	//Encode the grey image 
	const char* filename2 = "img/imtest.png";
	LodePNGColorType colortype = LCT_GREY;
	unsigned bitdepth = 8;
	//LodePNGColorType colortype = LCT_RGB;

	unsigned error3 = lodepng_encode_file(filename2, imggrey, width / 4, height / 4, colortype, bitdepth);
	if (error3) std::cout << "encoder error" << error3 << ":" << lodepng_error_text(error3) << std::endl;

	
	//Encode the disparity image 
	const char* filename4 = "img/disparity.png";
	const char* name_disp_right = "img/disparity_right.png";
	error3 = lodepng_encode_file(filename4, disparity_image, width / 4, height / 4, colortype, bitdepth);
	if (error3) std::cout << "encoder error" << error3 << ":" << lodepng_error_text(error3) << std::endl;
	error3 = lodepng_encode_file(name_disp_right, disparity_image_right, width / 4, height / 4, colortype, bitdepth);
	if (error3) std::cout << "encoder error" << error3 << ":" << lodepng_error_text(error3) << std::endl;
	//Encode Cross_checking
	const char* cross_file = "img/cross.png"; 
	error3 = lodepng_encode_file(cross_file, crosscheck, width / 4, height / 4, colortype, bitdepth);


	//Encode image 32 
	/*
	const char * filename3 = "img/imtest2.png";
	unsigned error4 = lodepng_encode32_file(filename3,imgreduct, width/4, height/4);
	if (error4)std::cout << "encoder error" << error4 << ":" << lodepng_error_text(error4) << std::endl;
	return 0;
*/
}