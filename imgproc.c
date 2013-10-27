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
}

void bayer(pixel **image, int width, int height)
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


	// (0, height)----------------------------(width, height)
	//	|											|
	//	|											|
	//	|											|
	// (0, 0)---------------------------------(width, 0)
	// R G R G R G
	// G B G B G B
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






