#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif


/*
   Read a BMP file, skip over the header, perform operation(s)
   on pixels, then put the header back on. Only work on 24-bit
   pixel BMP files.
*/

typedef struct {
   unsigned short int type;                 /* Magic identifier            */
   unsigned int size;                       /* File size in bytes          */
   unsigned short int reserved1, reserved2;
   unsigned int offset;                     /* Offset to image data, bytes */
} HEADER;

typedef struct {
   unsigned int size;               /* Header size in bytes      */
   int width,height;                /* Width and height of image */
   unsigned short int planes;       /* Number of colour planes   */
   unsigned short int bits;         /* Bits per pixel            */
   unsigned int compression;        /* Compression type          */
   unsigned int imagesize;          /* Image size in bytes       */
   int xresolution,yresolution;     /* Pixels per meter          */
   unsigned int ncolours;           /* Number of colours         */
   unsigned int importantcolours;   /* Important colours         */
} INFOHEADER;

typedef struct {
   unsigned char r,g,b,junk;
} COLOURINDEX;

typedef struct
{
	unsigned char r, g, b;
} pixel;

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



int main(int argc,char **argv)
{
	int				i,j					= 0;
	int				tmp					= 0;
	int				gotindex			= FALSE;
	unsigned char	r, g, b, pad		= '\0';
	int				padSize				= 0;
	char			*buf				= NULL;
	HEADER			header				= {0};
	INFOHEADER		infoheader			= {0};
	COLOURINDEX		colourindex[256]	= {0};
	
	FILE			*fptr		= NULL;
	FILE			*oFile		= NULL;

	pixel			**image		= NULL;
	
	// Check arguments
	if (argc != 2) {
		fprintf(stderr,"Usage: %s inFilename\n",argv[0]);
		fprintf(stderr,"Available operations:\n");
		exit(-1);
	}

	// Open file
	if ((fptr = fopen(argv[1],"r+b")) == NULL) {
		fprintf(stderr,"Unable to open BMP file \"%s\"\n",argv[1]);
		exit(-1);
	}

	if ((oFile = fopen("outImg.bmp","wb")) == NULL) {
		fprintf(stderr,"Unable to open output file \"outImg.bmp\"\n");
		exit(-1);
	}

	// Read and check the header
	fread (&header.type, 1, sizeof (header.type), fptr);
	fprintf(stderr,"ID is: %d, should be %d\n",header.type,'M'*256+'B');
	fread (&header.size, 1, sizeof (header.size), fptr);
	fprintf(stderr,"File size is %d bytes\n",header.size);
	fread (&header.reserved1, 1, sizeof (header.reserved1), fptr);
	fread (&header.reserved2, 1, sizeof (header.reserved2), fptr);
	fread (&header.offset, 1, sizeof (header.offset), fptr);
	fprintf(stderr,"Offset to image data is %d bytes\n",header.offset);

	// Read and check the information header
	if (fread(&infoheader,sizeof(INFOHEADER),1,fptr) != 1) {
		fprintf(stderr,"Failed to read BMP info header\n");
		exit(-1);
	}

	fprintf(stderr,"Image size = %d x %d\n",infoheader.width,infoheader.height);
	fprintf(stderr,"Number of colour planes is %d\n",infoheader.planes);
	fprintf(stderr,"Bits per pixel is %d\n",infoheader.bits);
	fprintf(stderr,"Compression type is %d\n",infoheader.compression);
	fprintf(stderr,"Number of colours is %d\n",infoheader.ncolours);
	fprintf(stderr,"Number of required colours is %d\n",
		infoheader.importantcolours);


	/* Read the lookup table if there is one */
	for (i=0;i<255;i++)
	{
		colourindex[i].r = rand() % 256;
		colourindex[i].g = rand() % 256;
		colourindex[i].b = rand() % 256;
		colourindex[i].junk = rand() % 256;
	}
	if (infoheader.ncolours > 0)
	{
		printf("DON'T!!!\n");
		exit(-1);

		for (i=0;i<infoheader.ncolours;i++)
		{
			if (fread(&colourindex[i].b,sizeof(unsigned char),1,fptr) != 1)
			{
				fprintf(stderr,"Image read failed\n");
				exit(-1);
			}
			if (fread(&colourindex[i].g,sizeof(unsigned char),1,fptr) != 1)
			{
				fprintf(stderr,"Image read failed\n");
				exit(-1);
			}
			if (fread(&colourindex[i].r,sizeof(unsigned char),1,fptr) != 1)
			{
				fprintf(stderr,"Image read failed\n");
				exit(-1);
			}
			if (fread(&colourindex[i].junk,sizeof(unsigned char),1,fptr) != 1)
			{
				fprintf(stderr,"Image read failed\n");
				exit(-1);
			}
				fprintf(stderr,"%3d\t%3d\t%3d\t%3d\n",i,
						colourindex[i].r,colourindex[i].g,colourindex[i].b);
		}
		gotindex = TRUE;
	}

	// Seek to the start of the image data
	fseek(fptr, header.offset, SEEK_SET);

	image = (pixel **)malloc(sizeof(pixel *) * infoheader.width);

	for (i = 0; i < infoheader.width; i++)
	{
		image[i] = (pixel *)malloc(sizeof(pixel) * infoheader.height);
	}

	// Find out how many bytes of paddings
	while ( ( infoheader.width*3 + padSize ) % 4 != 0 )
	{
		padSize++;
	}

	// Read the image 
	for (j = 0; j < infoheader.height; j++)
	{
		for (i = 0; i < infoheader.width; i++)
		{
			if (infoheader.bits != 24)
			{
				fprintf(stderr, "not 24-bit RGB pixel, exiting\n");
				return -1;
			}

			if (fread(&b, sizeof(unsigned char), 1, fptr) != 1)
			{
				fprintf(stderr,"Image read b failed\n");
				exit(-1);
			}
			if (fread(&g, sizeof(unsigned char), 1, fptr) != 1)
			{
				fprintf(stderr,"Image read g failed\n");
				exit(-1);
			}
			if (fread(&r, sizeof(unsigned char), 1, fptr)  != 1)
			{
				fprintf(stderr,"Image read r failed\n");
				exit(-1);
			}

			// Record image
			image[i][j].b	= b;
			image[i][j].g	= g;
			image[i][j].r	= r;
		}

		for (i = 0; i < padSize; i++)
		{
			if (fread(&pad, sizeof(unsigned char), 1, fptr)  != 1)
			{
				fprintf(stderr,"Image read pad failed\n");
				exit(-1);
			}
		}
	}
	
	// Operation function
	//add50(image, infoheader.width, infoheader.height);
	//emboss(image, infoheader.width, infoheader.height);
	//takeleft(image, infoheader.width, infoheader.height);
	sharpen(image, infoheader.width, infoheader.height);
	
	buf = (char *)malloc(sizeof(char) * header.offset);
	fclose(fptr);
	
	if ((fptr = fopen(argv[1],"r+b")) == NULL) {
		fprintf(stderr,"Unable to open BMP file \"%s\" the 2nd time\n",argv[1]);
		exit(-1);
	}

	if (fread(buf, sizeof(char) * header.offset, 1, fptr) != 1)
	{
		fprintf(stderr,"copy header failed\n");
		exit(-1);
	}

	fclose(fptr);

	// Write the output file
	tmp = fwrite(buf, sizeof(char), header.offset, oFile);
	if (tmp != header.offset)
	{
		fprintf(stderr, "%d written to oFile, but the offset should be %d", tmp, header.offset);
		exit(-1);	
	}
	
	for (j = 0; j < infoheader.height; j++)
	{
		for (i = 0; i < infoheader.width; i++)
		{
			fwrite(&image[i][j].b, sizeof(unsigned char), 1, oFile); 
			fwrite(&image[i][j].g, sizeof(unsigned char), 1, oFile);
			fwrite(&image[i][j].r, sizeof(unsigned char), 1, oFile);
		}

		for (i = 0; i < padSize; i++)
		{
			if (fwrite(&pad, sizeof(unsigned char), 1, oFile)  != 1)
			{
				fprintf(stderr,"Image read pad failed\n");
				exit(-1);
			}
		}

	}

	for (i = 0; i < infoheader.width; i++)
	{
		free(image[i]);
	}
	free(image);
	fclose(oFile);

	printf("Press enter to exit...\n");
	scanf("%c", buf);
}

