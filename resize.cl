
__kernel void change_size(read_only image2d_t imagein, write_only image2d_t imgreduct) {


	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	int w = get_global_id(0);
	int h = get_global_id(1);

	int2 coord_input = (int2) (w, h);
	
	
	if (h % 4 == 0 && w % 4 == 0) {
		
		uint4 pix = read_imageui(imagein, sampler, coord_input);
		int2 coord_output = (int2) (w/4, h/4);
		write_imageui(imgreduct, coord_output,pix);
	}
}

__kernel void to_grey(read_only image2d_t imagein, write_only  image2d_t imggrey) {
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

	int h = get_global_id(1);
	int w = get_global_id(0);
	uint r, g, b, a, grey = 0;
	int2 coord_input = (int2) (w, h);

	uint4 pix = read_imageui(imagein, sampler, coord_input);
	r = pix.s0;
	g = pix.s1;
	b = pix.s2;

	uint4 grey_output;
	grey = 0.2126*r + 0.7152*g + 0.0722*b;
	
	grey_output.s0 = grey;
	grey_output.s1 = grey;
	grey_output.s2 = grey;
	grey_output.s3 =grey;

	write_imageui(imggrey, coord_input, grey_output);
}
	
__kernel void ZNCC_image0(read_only image2d_t imageR, read_only image2d_t imageL, int disp_max, int win_size,write_only image2d_t output ) {
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	int i= get_global_id(1); //height
	int j = get_global_id(0); //width
	int2 dimensions = get_image_dim(imageR); //witdh, height 
	int win_half_value = 0.5 * (win_size - 1);

	float max_sum = 0;
	int best_d=0;
	int meanR;
	int meanL;
	float ZNCC_value;
	

	//if (j >= win_half_value && j < dimensions.y - win_half_value) {
	//	if (i >= win_half_value && i < dimensions.x - win_half_value) {

			for (int d = 0; d < disp_max; d++) {
				meanR = 0; //mean image right
				meanL = 0; //mean image left

				//if (d == 0) { printf("d egal 0"); }
				for (int win_y = -win_half_value; win_y <= win_half_value; win_y++) {
					for (int win_x = -win_half_value; win_x <= win_half_value; win_x++) {

							//int2 coordR = (int2) (i + win_x - d, dimensions.x*(j + win_y));
							//int2 coordL = (int2) (i + win_x, dimensions.x*(j + win_y));
							int2 coordR = (int2) ((j + win_x - d), (i + win_y));
							int2 coordL = (int2) ((j + win_x), (i + win_y));
							uint4 pixR = read_imageui(imageR, sampler, coordR);
							uint4 pixL = read_imageui(imageL, sampler, coordL);
							
							meanR += (int)(pixR.x);
							meanL += (int)(pixL.x);


							if (j == win_half_value+65 && i == win_half_value) {
								printf("coord %d, %d; pix_value %d; pixlvalue %d; meanR %d ; meanL %d; d %d; win half %d \n ", coordR.x, coordR.y, pixR.x, pixL.x, meanR, meanL, d,win_half_value);
							}

							//meanR += int(imageR[width*(j + win_y) + i + win_x - d]);
							//meanL += int(imageL[width*(j + win_y) + i + win_x]);

					}
				}
				meanR = meanR / (win_size*win_size);
				meanL = meanL / (win_size*win_size);

				int sum_mean = 0;
				float sum_varR = 0;
				float sum_varL = 0;
				for (int win_y = -win_half_value; win_y <= win_half_value; win_y++) {
					for (int win_x = -win_half_value; win_x <= win_half_value; win_x++) {

						//int2 coordR = (int2) (i + win_x - d, dimensions.x*(j + win_y));
						//int2 coordL = (int2) (i + win_x, dimensions.x*(j + win_y));
						int2 coordR = (int2) (j + win_x - d, i + win_y);
						int2 coordL = (int2) (j + win_x, i + win_y);
						uint4 pixR = read_imageui(imageR, sampler, coordR);
						uint4 pixL = read_imageui(imageL, sampler, coordL);


						sum_mean += ((int)pixL.x - meanL)*((int)pixL.x -meanR);
						sum_varL += ((int)pixL.x - meanL)*((int)pixL.x -meanL);
						sum_varR += ((int)pixL.x - meanR)*((int)pixL.x - meanR);

						//sum_mean += (imageL[width*(j + win_y) + i + win_x] - meanL)*(imageR[width*(j + win_y) + i + win_x - d] - meanR);
						//sum_varL += (imageL[width*(j + win_y) + i + win_x] - meanL)*(imageL[width*(j + win_y) + i + win_x] - meanL);
						//sum_varR += (imageR[width*(j + win_y) + i + win_x - d] - meanR)*(imageR[width*(j + win_y) + i + win_x - d] - meanR);
					}
				}
				ZNCC_value = sum_mean / (sqrt(sum_varR)*sqrt(sum_varL));

				if (ZNCC_value > max_sum) {
					max_sum = ZNCC_value;
					best_d = d;
				}
			}
			uint4 pix_out;
			pix_out.xyzw = best_d;
			int2 coord_out= (int2) (j, i);
			write_imageui(output, coord_out, pix_out);

			//int2 output_dim = get_image_dim(output);
			//print("image dimension ¨%d %d \n", output_dim.x, output_dim.y);
	//	}
	//}

}

