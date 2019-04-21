
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
	


