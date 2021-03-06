#include "stdio.h"
#include "stdlib.h"
#include <math.h>
#include "string.h"

#include "header\rgb_basic.h"


/*
   Read a BMP file, skip over the header, perform operation(s)
   on pixels, then put the header back on. Only work on 24-bit
   pixel BMP files.
*/


// Must have opened valid file pointers
void dump_pixel(pixel **image, int width, int height, FILE *a_out, FILE *b_out, FILE *c_out, int dumpRGB)
{
	int				i, j	= 0;
	int				x, y, z = '\0';
	
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			if (dumpRGB == 1)
			{
				x = (int)(image[i][j].r);
				y = (int)(image[i][j].g);
				z = (int)(image[i][j].b);
			}
			else
			{
				x = image[i][j].y;
				y = image[i][j].cb;
				z = image[i][j].cr;
			}

			// Integer
			fprintf(a_out, "%d\n", x);
			fprintf(b_out, "%d\n", y);
			fprintf(c_out, "%d\n", z);
		}
	}
}

void rgb2ycc (pixel **image, int width, int height)
{
	int		i,j			= 0;
	int		y,cb,cr		= 0;
	FILE	*yOut, *cbOut, *crOut = NULL;

	if ((yOut = fopen("output\\yOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"yOut\"\n");
		exit(-1);
	}
	
	if ((cbOut = fopen("output\\cbOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"cbOut\"\n");
		exit(-1);
	}
	if ((crOut = fopen("output\\crOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"crOut\"\n");
		exit(-1);
	}

	/*
	Y     0.2988   0.5869   0.1143        R

	Cb  = -0.1689  -0.3311  0.5000    X   G

	Cr    0.5000   -0.4189  -0.0811       B
	
	17 bits after the decimal point

	18'sd39164,  18'sd76926,  18'sd14982,
	-18'sd22138, -18'sd43398, 18'sd65536,
	18'sd65536, -18'sd54906, -18'sd10630
	*/
	
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
 			y	= ((int)image[i][j].r)*39164 + ((int)image[i][j].g)*76926 + ((int)image[i][j].b)*14982;
			cb	= ((int)image[i][j].r)*(-22138) - ((int)image[i][j].g)*43398 + ((int)image[i][j].b)*65536;
			cr	= ((int)image[i][j].r)*65536 - ((int)image[i][j].g)*54906 - ((int)image[i][j].b)*10630;

			y	= y >> 8;
			cb	= cb >> 8;
			cr	= cr >> 8;

			image[i][j].y	= y;
			image[i][j].cb	= cb;
			image[i][j].cr	= cr;
		}
	}

	dump_pixel(image, width, height, yOut, cbOut, crOut, 0);

	fclose(yOut);
	fclose(cbOut);
	fclose(crOut);
}

void ycclut (pixel **image, int width, int height)
{
	int			i,j			= 0;
	double		y_orig		= 0;
	double		y_double	= 0;
	long long	y_longlong	= 0;
	double		rms			= 0;
	double		diff		= 0;
	double		max_diff	= 0;
	double		min_diff	= 0;

	double		diff_percent = 0;
	double		max_diff_percent = 0;

	double		y_range	= 0;

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
 			y_orig		= ((double)image[i][j].y)/512;
			y_double	= pow(y_orig, (double)0.4);
			y_longlong	= (long long)(y_double*((double)32));
			y_longlong	= y_longlong & 0x1FFF;

			diff = ((double)y_double-((double)y_longlong/(double)32));

			if (diff > max_diff)
			{
				max_diff = diff;				
			}

			diff_percent = (diff/y_double);
			if (diff_percent > max_diff_percent)
			{
				max_diff_percent = diff_percent;
			}

			rms += diff*diff;
		}
	}
	rms /= (height*width);
	rms = pow(rms, 0.5);


	printf("\nrms is %f\nmax diff = %f, max diff percentage = %f\%\n\n",
			rms, max_diff, max_diff_percent);

	max_diff	= 0;
	min_diff	= 0;
	for (y_range = 0; y_range <= 255; y_range += 0.0625)
	{
		diff = y_range - pow(y_range, 0.4);
		
		if (diff > max_diff)
		{
			max_diff = diff;				
		}
		if (diff < min_diff)
		{
			min_diff = diff;				
		}
	}

	printf("On gamma curve, max_diff = %f, min_diff = %f\n", max_diff, min_diff);

}

void ycc2rgb (pixel **image, int width, int height)
{
	int			i,j			= 0;
	long long	r,g,b		= 0;
	int			mask		= 0x00FF;
	FILE		*rOut, *gOut, *bOut = NULL;

	if ((rOut = fopen("output\\rOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"rOut\"\n");
		exit(-1);
	}
	
	if ((gOut = fopen("output\\gOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"gOut\"\n");
		exit(-1);
	}
	if ((bOut = fopen("output\\bOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"bOut\"\n");
		exit(-1);
	}

	/*
	R     1   0	       1.402        Y

	G  =  1  -0.3441  -0.7141   X   Cb

	B     1   1.772    0.00015      Cr
	
	16 bits after the decimal point

	18'sd65536,  18'sd0,       18'sd91881,
	18'sd65536, -18'sd22551,  -18'sd46799,
	18'sd65536,  18'sd112853,  18'sd10
	*/
	
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			r	= ((long long)image[i][j].y)*(long long)65536  + ((long long)image[i][j].cb)*(long long)0       + ((long long)image[i][j].cr)*(long long)91881;
			g	= ((long long)image[i][j].y)*(long long)65536 - ((long long)image[i][j].cb)*(long long)22551  - ((long long)image[i][j].cr)*(long long)46799;
			b	= ((long long)image[i][j].y)*(long long)65536 + ((long long)image[i][j].cb)*(long long)112853 + ((long long)image[i][j].cr)*(long long)10;

			r = (r >> 25) & mask;
			g = (g >> 25) & mask;
			b = (b >> 25) & mask;

			image[i][j].r	= (unsigned char)r;
			image[i][j].g	= (unsigned char)g;
			image[i][j].b	= (unsigned char)b;
		}
	}

	dump_pixel(image, width, height, rOut, gOut, bOut, 1);

	fclose(rOut);
	fclose(gOut);
	fclose(bOut);
}

void conv_2d(int **kernel, int kernelSize, pixel **image, int width, int height, double factor, int bias)
{
	int i, j	= 0;	// Image iterator
	int m, n	= 0;	// kernel iterator

	int r, g, b	= 0;	// RGB result values
	int x, y	= 0;	// result coordinate

	pixel **result = NULL;

	result = (pixel **)malloc(sizeof(pixel *) * width);

	for (i = 0; i < width; i++)
	{
		result[i] = (pixel *)malloc(sizeof(pixel) * height);
	}

	// Scan through the image
	for(i = 0; i < width; i++) 
	{
		// Debugging...
		if (i == 80)
		{
			i = 80;
		}
		for(j = 0; j < height; j++) 
		{ 
			r = 0;
			g = 0;
			b = 0; 
         
			// multiply every value of the filter with corresponding image pixel (i, j)
			//		n	-->
			//	m	a		b		c
			//	|	d		e		f
			//	|	g		h		i
			//	\/
			
			for (m = 0; m < kernelSize; m++)		// row
			{
				for (n = 0; n < kernelSize; n++)	// column
				{
					x = i - kernelSize/2 + m; 
					y = j - kernelSize/2 + n;

					// If the result is out of the image area, don't do anything
					if (	x >= 0 && x < width
						&&	y >= 0 && y < height)
					{	
						r += image[x][y].r * kernel[n][m];
						g += image[x][y].g * kernel[n][m];
						b += image[x][y].b * kernel[n][m];
					}
				}
			}

			r = (int)(factor * r + bias);
			g = (int)(factor * g + bias);
			b = (int)(factor * b + bias);

			//truncate values smaller than zero and larger than 255 
			result[i][j].r = (unsigned char)((r < 0) ? 0 : ((r > 255) ? 255 : r));
			result[i][j].g = (unsigned char)((g < 0) ? 0 : ((g > 255) ? 255 : g));
			result[i][j].b = (unsigned char)((b < 0) ? 0 : ((b > 255) ? 255 : b));
        }
    }

	for(i = 0; i < width; i++) 
	{
		for(j = 0; j < height; j++) 
		{
			image[i][j].r = result[i][j].r;
			image[i][j].g = result[i][j].g;
			image[i][j].b = result[i][j].b;
		}
	}
	for(i = 0; i < width; i++) 
	{
		free(result[i]);
	}
	free(result);
}

// take the left pixel
void takeleft(pixel **image, int width, int height)
{
	int i, j = 0;
	for(i = width - 1; i > 0; i--) 
	{
		for(j = 0; j < height; j++) 
		{ 
			image[i][j].r = image[i-1][j].r;
			image[i][j].g = image[i-1][j].g;
			image[i][j].b = image[i-1][j].b;
		}
	}
}

// Operation for test, add 50 to all rgb values
void add50(pixel **image, int width, int height)
{
	int i, j = 0;
	for(i = 0; i < width; i++) 
	{
		for(j = 0; j < height; j++) 
		{ 
			image[i][j].r = (unsigned char)(((int)image[i][j].r + 50)%255);
			image[i][j].g = (unsigned char)(((int)image[i][j].g + 50)%255);
			image[i][j].b = (unsigned char)(((int)image[i][j].b + 50)%255);
		}
	}
}

void emboss(pixel **image, int width, int height)
{
	int		**kernel	= NULL;
	int		kernelSize	= 3;
	
	int		i			= 0;
	double	factor		= 1.0;
	int		bias		= 128;


	kernel = (int **)malloc(sizeof(int *) * kernelSize);
	
	for (i = 0; i < kernelSize; i++)
	{
		kernel[i] = (int *)malloc(sizeof(int) * kernelSize);
		memset(kernel[i], 0, sizeof(int) * kernelSize);
	}

	/*
		 -1, -1,  0,
		 -1,  0,  1,
		 0,  1,  1
	*/
	kernel[0][0] = -1;
	kernel[0][1] = -1;
	kernel[1][0] = -1;

	kernel[1][2] = 1;
	kernel[2][1] = 1;
	kernel[2][2] = 1;
	
	conv_2d(kernel, kernelSize, image, width, height, factor, bias);

	for (i = 0; i < kernelSize; i++)
	{
		free(kernel[i]);
	}
	free(kernel);
}

void sharpen(pixel **image, int width, int height)
{
	int		**kernel	= NULL;
	int		kernelSize	= 3;
	
	int		i			= 0;
	double	factor		= 1.0;
	int		bias		= 0;

	FILE	*sharpenROut, *sharpenGOut, *sharpenBOut = NULL;

	if ((sharpenROut = fopen("output\\sharpenROut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"sharpenROut\"\n");
		exit(-1);
	}
	
	if ((sharpenGOut = fopen("output\\sharpenGOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"sharpenGOut\"\n");
		exit(-1);
	}
	if ((sharpenBOut = fopen("output\\sharpenBOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"sharpenBOut\"\n");
		exit(-1);
	}

	kernel = (int **)malloc(sizeof(int *) * kernelSize);
	
	for (i = 0; i < kernelSize; i++)
	{
		kernel[i] = (int *)malloc(sizeof(int) * kernelSize);
		memset(kernel[i], 0, sizeof(int) * kernelSize);
	}

	/*
		-1, -1, -1,
		-1,  9, -1,
		-1, -1, -1
	*/
	kernel[0][0] = -1;
	kernel[0][1] = -1;
	kernel[0][2] = -1;

	kernel[1][0] = -1;
	kernel[1][1] = 9;
	kernel[1][2] = -1;

	kernel[2][0] = -1;
	kernel[2][1] = -1;
	kernel[2][2] = -1;
	
	conv_2d(kernel, kernelSize, image, width, height, factor, bias);

	for (i = 0; i < kernelSize; i++)
	{
		free(kernel[i]);
	}
	free(kernel);

	dump_pixel(image, width, height, sharpenROut, sharpenGOut, sharpenBOut, 1);

	fclose(sharpenROut);
	fclose(sharpenGOut);
	fclose(sharpenBOut);

}

void bayer(pixel **image, int width, int height)
{
	int i, j	= 0;	// Image iterator
	int r, g, b	= 0;	// RGB result values

	pixel **result = NULL;

	result = (pixel **)malloc(sizeof(pixel *) * width);

	for (i = 0; i < width; i++)
	{
		result[i] = (pixel *)malloc(sizeof(pixel) * height);
	}


	// (0, height)----------------------------(width, height)
	//	|											|
	//	|											|
	//	|											|
	// (0, 0)---------------------------------(width, 0)
	//
	// .
	// .
	// R G R G R G
	// G B G B G B . . . . 
	for (i = width - 1; i >= 0; i--)
	{
		for (j = height - 1; j >= 0; j--)
		{
			if (	((width-i-1) % 2) == 0 && ((height-j-1) % 2 == 0)
				||	((width-i-1) % 2) == 1 && ((height-j-1) % 2 == 1))
			{
				// G
				result[i][j].r = 0;
				result[i][j].g = image[i][j].g;
				result[i][j].b = 0;
			}
			else if (((width-i-1) % 2) == 1 && ((height-j-1) % 2 == 0))
			{
				// R
				result[i][j].r = image[i][j].r;
				result[i][j].g = 0;
				result[i][j].b = 0;
			}
			else
			{
				// B
				result[i][j].r = 0;
				result[i][j].g = 0;
				result[i][j].b = image[i][j].b;
			}
		}	
	}


	for(i = 0; i < width; i++) 
	{
		for(j = 0; j < height; j++) 
		{
			image[i][j].r = result[i][j].r;
			image[i][j].g = result[i][j].g;
			image[i][j].b = result[i][j].b;
		}
	}
	for(i = 0; i < width; i++) 
	{
		free(result[i]);
	}
	free(result);

}


void demosaic_neighbor(pixel **image, int width, int height)
{
	int i, j	= 0;	// Image iterator
	int r, g, b	= 0;	// RGB result values
	FILE	*demosaicROut, *demosaicGOut, *demosaicBOut = NULL;

	pixel **result = NULL;

	result = (pixel **)malloc(sizeof(pixel *) * width);

	for (i = 0; i < width; i++)
	{
		result[i] = (pixel *)malloc(sizeof(pixel) * height);
	}


	// (0, height)----------------------------(width, height)
	//	|											|
	//	|											|
	//	|											|
	// (0, 0)---------------------------------(width, 0)
	//
	// .
	// .
	// R G R G R G
	// G B G B G B . . . . 
	for (i = 0; i < width; i++)
	{
		// Deal with the last row later
		for (j = 0; j < height - 1; j++)
		{
			if (	(i % 2 == 0)
				&&	(j % 2 == 0))
			{
				if (i == 0)
				{					
					// G from the previous line, it's actually the
					// last element(B) of this row
					g = ((int)(image[width-1][j].b) + (int)(image[i][j].g))/2;
					
					if (j == 0)
					{
						// B in front of the very first pixel, let it be 0
						b = 0;
					}
					else
					{
						// The B in front of the first pixel in this row
						// is the G of the previous row
						b = image[width-1][j-1].g;
					}
				}
				else
				{
					g = (image[i-1][j+1].g + image[i][j].g)/2;	
					b = image[i-1][j].b;
				}

				r = image[i][j+1].r;

				result[i][j].r = (unsigned char)r;
				result[i][j].g = (unsigned char)g;
				result[i][j].b = (unsigned char)b;
			}
			else if (		(i % 2 == 1)
						&&	(j % 2 == 0))
			{
				r = image[i-1][j+1].r;
				g = ((int)(image[i-1][j].g) + (int)(image[i][j+1].g))/2;
				b = image[i][j].b;
			
				result[i][j].r = (unsigned char)r;
				result[i][j].g = (unsigned char)g;
				result[i][j].b = (unsigned char)b;
			}
			else if (		(i % 2 == 0)
						&&	(j % 2 == 1))
			{
				if (i == 0)
				{
					// G is the avg of the last element of previous row,
					// which is a B, and the one about the current pixel
					g = ((int)(image[width-1][j-1].b) + (int)(image[i][j+1].g))/2;
				
					// B is the one in front of the G about current pixel,
					// i.e. the G at the end of this row
					b = image[width-1][j].g;
				}
				else
				{
					g = ((int)(image[i-1][j].g) + (int)(image[i][j+1].g))/2;
					b = image[i-1][j+1].b;
				}

				r = image[i][j].r;

				result[i][j].r = (unsigned char)r;
				result[i][j].g = (unsigned char)g;
				result[i][j].b = (unsigned char)b;
			}
			else	//(i % 2 == 1) && (j % 2 == 1))
			{
				r = image[i-1][j].r;
				g = ((int)(image[i][j].g) + (int)(image[i-1][j+1].g))/2;
				b = image[i][j+1].b;
				
				result[i][j].r = (unsigned char)r;
				result[i][j].g = (unsigned char)g;
				result[i][j].b = (unsigned char)b;
			}
		}
	}

	// The last row
	for (i = 0; i < width; i++)
	{
		if (i % 2 == 0)
		{
			if (i == 0)
			{
				b = image[width-1][height-1].g;
				g = ((int)(image[width-1][height-2].b))/2;
			}
			else
			{
				b = 0;
				g = ((int)image[i-1][height-1].g)/2;
			}
			r = image[i][height-1].r;
		}
		else
		{
			r = image[i-1][height-1].r;
			g = ((int)image[i][j].g)/2;
			b = 0;
		}

		result[i][j].r = (unsigned char)r;
		result[i][j].g = (unsigned char)g;
		result[i][j].b = (unsigned char)b;

	}

	for(i = 0; i < width; i++) 
	{
		for(j = 0; j < height; j++) 
		{
			image[i][j].r = result[i][j].r;
			image[i][j].g = result[i][j].g;
			image[i][j].b = result[i][j].b;
		}
	}
	for(i = 0; i < width; i++) 
	{
		free(result[i]);
	}
	free(result);

	if ((demosaicROut = fopen("output\\demosaicROut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"demosaicROut\"\n");
		exit(-1);
	}
	
	if ((demosaicGOut = fopen("output\\demosaicGOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"demosaicGOut\"\n");
		exit(-1);
	}
	if ((demosaicBOut = fopen("output\\demosaicBOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"demosaicBOut\"\n");
		exit(-1);
	}

	dump_pixel(image, width, height, demosaicROut, demosaicGOut, demosaicBOut, 1);

	fclose(demosaicROut);
	fclose(demosaicGOut);
	fclose(demosaicBOut);
}


void demosaic_acpi(pixel **image, int width, int height)
{
	int i, j	= 0;	// Image iterator
	int r, g, b	= 0;	// RGB result values
	int	h, v	= 0;	// Gradient
	int	gh, gv, ghv	= 0;
	int	f, T	= 0;
	pixel	rf[9] = {0};
	int	g28, g46, g19, g37 = 0;
	int n, p	= 0;
	int	tmp1, tmp2 	= 0;
	FILE	*demosaicROut, *demosaicGOut, *demosaicBOut = NULL;

	pixel **result = NULL;

	result = (pixel **)malloc(sizeof(pixel *) * width);

	for (i = 0; i < width; i++)
	{
		result[i] = (pixel *)malloc(sizeof(pixel) * height);
	}

	f = 0;
	// (0, height)----------------------------(width, height)
	//	|											|
	//	|											|
	//	|											|
	// (0, 0)---------------------------------(width, 0)
	//
	// .
	// .
	// R G R G R G
	// G B G B G B . . . . 

	// Green interpolation
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			// Calculate gradients
			if (i == 0)
			{
				// First column
				h = (int)image[i+1][j].g;
			}
			else if (i == width-1)
			{
				// Last column
				h = (int)image[i-1][j].g;
			}
			else
			{
				h = abs((int)image[i+1][j].g - (int)image[i-1][j].g);
			}

			if (j == 0)
			{
				// First row
				v = (int)image[i][j+1].g;			
			}
			else if (j == height-1)
			{
				// Last row
				v = (int)image[i][j-1].g;
			}
			else
			{
				v = abs((int)image[i][j+1].g - (int)image[i][j-1].g);
			}

			// Calculate gh, gv and gvh
			if (j == 0)
			{
				// First row
				gv = (int)image[i][j+1].g;			
			}
			else if (j == height-1)
			{
				// Last row
				gv = (int)image[i][j-1].g;
			}
			else
			{
				gv = ((int)image[i][j+1].g + (int)image[i][j-1].g)/2;
			}

			if (i == 0)
			{
				// First column
				gh = (int)image[i+1][j].g;
			}
			else if (i == width-1)
			{
				// Last column
				gh = (int)image[i-1][j].g;
			}
			else
			{
				gh = ((int)image[i+1][j].g + (int)image[i-1][j].g)/2;
			}

			ghv = (gh + gv)/2;

			// Select interpolated g
			if (h > v)
			{
				g = gv;
			}
			else if (h < v)
			{
				g = gh;
			}
			else
			{
				g = ghv;
			}

			if (	((i % 2 == 0) &&	(j % 2 == 0))
				||	((i % 2 == 1) &&	(j % 2 == 1)))
			{
				// G at the center, no need to interpolate
				result[i][j].r = (unsigned char)0;
				result[i][j].g = (image[i][j].g);
				result[i][j].b = (unsigned char)0;
			}
			else 
			{
				f = f + h + v;
				if (		(i % 2 == 1)
						&&	(j % 2 == 0))
				{
					//	R	G	R
					//	G	B	G
					//	R	G	R
					result[i][j].r = (unsigned char)0;
					result[i][j].g = (unsigned char)g;
					result[i][j].b = image[i][j].b;
				}
				else if (		(i % 2 == 0)
							&&	(j % 2 == 1))
				{
					//	B	G	B
					//	G	R	G
					//	B	G	B
					result[i][j].r = image[i][j].r;
					result[i][j].g = (unsigned char)g;
					result[i][j].b = (unsigned char)0;
				}
			}
		}
	}

	// Determine threashold
	if (f < 73242)
	{
		T = 50;
	}
	else if (f < 102539)
	{
		T = 40;
	}
	else if (f < 146484)
	{
		T = 20;
	}
	else if (f < 292965)
	{
		T = 15;
	}
	else
	{
		T = 8;
	}

	// RG interpolation
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			// Insert 0's at the boundary
			if (i == 0)
			{
				// First column
				if (j == 0)
				{
					// First row
					memcpy(&rf[0], &result[i+1][j+1], sizeof(pixel));
					memcpy(&rf[1], &result[i][j+1], sizeof(pixel));
					memset(&rf[2], 0, sizeof(pixel));
					memcpy(&rf[3], &result[i+1][j], sizeof(pixel));
					memcpy(&rf[4], &result[i][j], sizeof(pixel));
					memset(&rf[5], 0, sizeof(pixel));
					memset(&rf[6], 0, sizeof(pixel));
					memset(&rf[7], 0, sizeof(pixel));
					memset(&rf[8], 0, sizeof(pixel));
				}
				else if (j == height - 1)
				{
					// Last row
					memset(&rf[0], 0, sizeof(pixel));
					memset(&rf[1], 0, sizeof(pixel));
					memset(&rf[2], 0, sizeof(pixel));
					memcpy(&rf[3], &result[i+1][j], sizeof(pixel));
					memcpy(&rf[4], &result[i][j], sizeof(pixel));
					memset(&rf[5], 0, sizeof(pixel));
					memcpy(&rf[6], &result[i+1][j-1], sizeof(pixel));
					memcpy(&rf[7], &result[i][j-1], sizeof(pixel));
					memset(&rf[8], 0, sizeof(pixel));
				}
				else
				{
					memcpy(&rf[0], &result[i+1][j+1], sizeof(pixel));
					memcpy(&rf[1], &result[i][j+1], sizeof(pixel));
					memset(&rf[2], 0, sizeof(pixel));
					memcpy(&rf[3], &result[i+1][j], sizeof(pixel));
					memcpy(&rf[4], &result[i][j], sizeof(pixel));
					memset(&rf[5], 0, sizeof(pixel));
					memcpy(&rf[6], &result[i+1][j-1], sizeof(pixel));
					memcpy(&rf[7], &result[i][j-1], sizeof(pixel));
					memset(&rf[8], 0, sizeof(pixel));
				}
			}
			else if (i == width-1)
			{
				// Last column
				if (j == 0)
				{
					// First row
					memset(&rf[0], 0, sizeof(pixel));
					memcpy(&rf[1], &result[i][j+1], sizeof(pixel));
					memcpy(&rf[2], &result[i-1][j+1], sizeof(pixel));
					memset(&rf[3], 0, sizeof(pixel));
					memcpy(&rf[4], &result[i][j], sizeof(pixel));
					memcpy(&rf[5], &result[i-1][j], sizeof(pixel));
					memset(&rf[6], 0, sizeof(pixel));
					memset(&rf[7], 0, sizeof(pixel));
					memset(&rf[8], 0, sizeof(pixel));
				}
				else if (j == height - 1)
				{
					// Last row
					memset(&rf[0], 0, sizeof(pixel));
					memset(&rf[1], 0, sizeof(pixel));
					memset(&rf[2], 0, sizeof(pixel));
					memset(&rf[3], 0, sizeof(pixel));
					memcpy(&rf[4], &result[i][j], sizeof(pixel));
					memcpy(&rf[5], &result[i-1][j], sizeof(pixel));
					memset(&rf[6], 0, sizeof(pixel));
					memcpy(&rf[7], &result[i][j-1], sizeof(pixel));
					memcpy(&rf[8], &result[i-1][j-1], sizeof(pixel));
				}
				else
				{
					memset(&rf[0], 0, sizeof(pixel));
					memcpy(&rf[1], &result[i][j+1], sizeof(pixel));
					memcpy(&rf[2], &result[i-1][j+1], sizeof(pixel));
					memset(&rf[3], 0, sizeof(pixel));
					memcpy(&rf[4], &result[i][j], sizeof(pixel));
					memcpy(&rf[5], &result[i-1][j], sizeof(pixel));
					memset(&rf[6], 0, sizeof(pixel));
					memcpy(&rf[7], &result[i][j-1], sizeof(pixel));
					memcpy(&rf[8], &result[i-1][j-1], sizeof(pixel));
				}
			}
			else
			{
				if (j == 0)
				{
					// First row
					memcpy(&rf[0], &result[i+1][j+1], sizeof(pixel));
					memcpy(&rf[1], &result[i][j+1], sizeof(pixel));
					memcpy(&rf[2], &result[i-1][j+1], sizeof(pixel));
					memcpy(&rf[3], &result[i+1][j], sizeof(pixel));
					memcpy(&rf[4], &result[i][j], sizeof(pixel));
					memcpy(&rf[5], &result[i-1][j], sizeof(pixel));
					memset(&rf[6], 0, sizeof(pixel));
					memset(&rf[7], 0, sizeof(pixel));
					memset(&rf[8], 0, sizeof(pixel));
				}
				else if (j == height - 1)
				{
					// Last row
					memset(&rf[0], 0, sizeof(pixel));
					memset(&rf[1], 0, sizeof(pixel));
					memset(&rf[2], 0, sizeof(pixel));
					memcpy(&rf[3], &result[i+1][j], sizeof(pixel));
					memcpy(&rf[4], &result[i][j], sizeof(pixel));
					memcpy(&rf[5], &result[i-1][j], sizeof(pixel));
					memcpy(&rf[6], &result[i+1][j-1], sizeof(pixel));
					memcpy(&rf[7], &result[i][j-1], sizeof(pixel));
					memcpy(&rf[8], &result[i-1][j-1], sizeof(pixel));
				}
				else
				{
					memcpy(&rf[0], &result[i+1][j+1], sizeof(pixel));
					memcpy(&rf[1], &result[i][j+1], sizeof(pixel));
					memcpy(&rf[2], &result[i-1][j+1], sizeof(pixel));
					memcpy(&rf[3], &result[i+1][j], sizeof(pixel));
					memcpy(&rf[4], &result[i][j], sizeof(pixel));
					memcpy(&rf[5], &result[i-1][j], sizeof(pixel));
					memcpy(&rf[6], &result[i+1][j-1], sizeof(pixel));
					memcpy(&rf[7], &result[i][j-1], sizeof(pixel));
					memcpy(&rf[8], &result[i-1][j-1], sizeof(pixel));
				}
			}

			// Gradiants
			g46 = abs((int)rf[3].g - (int)rf[5].g);
			g28 = abs((int)rf[1].g - (int)rf[7].g);
			g19 = abs((int)rf[0].g - (int)rf[8].g);
			g37 = abs((int)rf[2].g - (int)rf[6].g);

			if ((i % 2 == 0) &&	(j % 2 == 0))
			{
				//	G	R	G
				//	B	G	B
				//	G	R	G
				if ((g46 > T) || (g28 > T) || (g19 > T) || (g37 > T))
				{
					// Edge pixel
					result[i][j].r =
						(unsigned char)(((int)rf[1].r+(int)rf[7].r)/2 + ((2*(int)rf[4].g-(int)rf[1].g-(int)rf[7].g)>>1));
					result[i][j].b =
						(unsigned char)(((int)rf[3].b+(int)rf[5].b)/2 + ((2*(int)rf[4].g-(int)rf[3].g-(int)rf[5].g)>>1));
				}
				else
				{
					// Smooth
					result[i][j].r = (unsigned char)(((int)rf[1].r+(int)rf[7].r)/2);
					result[i][j].b = (unsigned char)(((int)rf[3].b+(int)rf[5].b)/2);
				}

				// G is unchanged
			}
			else if ((i % 2 == 1) && (j % 2 == 0))
			{
				//	R	G	R
				//	G	B	G
				//	R	G	R
				if ((g46 > T) || (g28 > T) || (g19 > T) || (g37 > T))
				{
					// Edge pixel
					n = abs((int)rf[0].r - (int)rf[8].r) + abs(2*(int)rf[4].g - (int)rf[0].g - (int)rf[8].g);
					p = abs((int)rf[2].r - (int)rf[6].r) + abs(2*(int)rf[4].g - (int)rf[2].g - (int)rf[6].g);

					tmp1 = (((int)rf[0].r+(int)rf[8].r)/2 + ((2*(int)rf[4].g-(int)rf[0].g-(int)rf[8].g)>>1));
					tmp2 = (((int)rf[2].r+(int)rf[6].r)/2 + ((2*(int)rf[4].g-(int)rf[2].g-(int)rf[6].g)>>1));
					
					if (n < p)
					{		
						result[i][j].r = (unsigned char)tmp1;
					}
					else if (n > p)
					{
						result[i][j].r = (unsigned char)tmp2;
					}
					else
					{
						result[i][j].r = ((unsigned char)tmp1 + (unsigned char)tmp2)/2;
					}

				}
				else
				{
					// Smooth
					if (g19 > g37)
					{
						result[i][j].r = (unsigned char)(((int)rf[2].r+(int)rf[6].r)/2);
					}
					else
					{
						result[i][j].r = (unsigned char)(((int)rf[0].r+(int)rf[8].r)/2);
					}
				}
				// G and B are unchanged
			}
			else if (		(i % 2 == 0)
						&&	(j % 2 == 1))
			{
				//	B	G	B
				//	G	R	G
				//	B	G	B
				if ((g46 > T) || (g28 > T) || (g19 > T) || (g37 > T))
				{
					// Edge pixel
					n = abs((int)rf[0].b - (int)rf[8].b) + abs(2*(int)rf[4].g - (int)rf[0].g - (int)rf[8].g);
					p = abs((int)rf[2].b - (int)rf[6].b) + abs(2*(int)rf[4].g - (int)rf[2].g - (int)rf[6].g);

					tmp1 = (((int)rf[0].b+(int)rf[8].b)/2 + ((2*(int)rf[4].g-(int)rf[0].g-(int)rf[8].g)>>1));
					tmp2 = (((int)rf[2].b+(int)rf[6].b)/2 + ((2*(int)rf[4].g-(int)rf[2].g-(int)rf[6].g)>>1));
					
					if (n < p)
					{		
						result[i][j].b = (unsigned char)tmp1;
					}
					else if (n > p)
					{
						result[i][j].b = (unsigned char)tmp2;
					}
					else
					{
						result[i][j].b = ((unsigned char)tmp1 + (unsigned char)tmp2)/2;
					}
				}
				else
				{
					// Smooth
					if (g19 > g37)
					{
						result[i][j].b = (unsigned char)(((int)rf[2].b+(int)rf[6].b)/2);
					}
					else
					{
						result[i][j].b = (unsigned char)(((int)rf[0].b+(int)rf[8].b)/2);
					}
				}
				
				// R and G are unchanged
			}
			else //((i % 2 == 1) &&	(j % 2 == 1))
			{
				//	G	B	G
				//	R	G	R
				//	G	B	G
				if ((g46 > T) || (g28 > T) || (g19 > T) || (g37 > T))
				{
					// Edge pixel
					result[i][j].r =
						(unsigned char)(((int)rf[3].r+(int)rf[5].r)/2 + ((2*(int)rf[4].g-(int)rf[3].g-(int)rf[5].g)>>1));
					result[i][j].b =
						(unsigned char)(((int)rf[1].b+(int)rf[7].b)/2 + ((2*(int)rf[4].g-(int)rf[1].g-(int)rf[7].g)>>1));
				}
				else
				{
					// Smooth
					result[i][j].r = (unsigned char)(((int)rf[3].r+(int)rf[5].r)/2);
					result[i][j].b = (unsigned char)(((int)rf[1].b+(int)rf[7].b)/2);
				}
				// G is unchanged
			}
		}
	}

	for(i = 0; i < width; i++) 
	{
		for(j = 0; j < height; j++) 
		{
			image[i][j].r = result[i][j].r;
			image[i][j].g = result[i][j].g;
			image[i][j].b = result[i][j].b;
		}
	}
	for(i = 0; i < width; i++) 
	{
		free(result[i]);
	}
	free(result);

	if ((demosaicROut = fopen("output\\demosaicROut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"demosaicROut\"\n");
		exit(-1);
	}
	
	if ((demosaicGOut = fopen("output\\demosaicGOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"demosaicGOut\"\n");
		exit(-1);
	}
	if ((demosaicBOut = fopen("output\\demosaicBOut","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"demosaicBOut\"\n");
		exit(-1);
	}

	dump_pixel(image, width, height, demosaicROut, demosaicGOut, demosaicBOut, 1);

	fclose(demosaicROut);
	fclose(demosaicGOut);
	fclose(demosaicBOut);
}






