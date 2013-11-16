#include "stdio.h"
#include "stdlib.h"
#include "math.h"
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
					b = image[i][j+1].b;
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



