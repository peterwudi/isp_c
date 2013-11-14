#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"

#include "header\rgb_basic.h"
#include "header\imgproc.h"

/*
	Read a BMP file
	Write out RGB, and binary form,
	Filter the image
*/

void bmpToRGB(char *inFilename, int proc)
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
	FILE			*rgbIntFile	= NULL;
	FILE			*rOrig		= NULL;
	FILE			*gOrig		= NULL;
	FILE			*bOrig		= NULL;

	FILE			*oFile		= NULL;
	pixel			**image		= NULL;


	// Check arguments
	if (inFilename == NULL) {
		fprintf(stderr,"inFilename ptr is NULL\n");
		exit(-1);
	}

	// Open file
	if ((fptr = fopen(inFilename,"r+b")) == NULL) {
		fprintf(stderr,"Unable to open BMP file \"%s\"\n",inFilename);
		exit(-1);
	}

	
	if ((rgbIntFile = fopen("output\\rgbIntFile","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"rgbIntFile\"\n");
		exit(-1);
	}
	if ((rOrig = fopen("output\\rOrig","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"rOrig\"\n");
		exit(-1);
	}
	if ((gOrig = fopen("output\\gOrig","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"gOrig\"\n");
		exit(-1);
	}
	if ((bOrig = fopen("output\\bOrig","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"bOrig\"\n");
		exit(-1);
	}

	if ((oFile = fopen("output\\outImg.bmp","wb")) == NULL) {
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
	for (i=0;i<255;i++) {
		colourindex[i].r = rand() % 256;
		colourindex[i].g = rand() % 256;
		colourindex[i].b = rand() % 256;
		colourindex[i].junk = rand() % 256;
	}
	if (infoheader.ncolours > 0) {
		for (i=0;i<infoheader.ncolours;i++) {
			if (fread(&colourindex[i].b,sizeof(unsigned char),1,fptr) != 1) {
			fprintf(stderr,"Image read failed\n");
			exit(-1);
			}
			if (fread(&colourindex[i].g,sizeof(unsigned char),1,fptr) != 1) {
			fprintf(stderr,"Image read failed\n");
			exit(-1);
			}
			if (fread(&colourindex[i].r,sizeof(unsigned char),1,fptr) != 1) {
			fprintf(stderr,"Image read failed\n");
			exit(-1);
			}
			if (fread(&colourindex[i].junk,sizeof(unsigned char),1,fptr) != 1) {
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
				return;
			}

			if (fread(&b, sizeof(unsigned char), 1, fptr) != 1) {
				fprintf(stderr,"Image read failed\n");
				exit(-1);
			}
			if (fread(&g, sizeof(unsigned char), 1, fptr) != 1) {
				fprintf(stderr,"Image read failed\n");
				exit(-1);
			}
			if (fread(&r, sizeof(unsigned char), 1, fptr)  != 1) {
				fprintf(stderr,"Image read failed\n");
				exit(-1);
			}

			// binary
			//fwrite (&r, 1 , sizeof(unsigned char) , rgbFile);
			//fwrite (&g, 1 , sizeof(unsigned char) , rgbFile);
			//fwrite (&b, 1 , sizeof(unsigned char) , rgbFile);

			//fwrite (&r , 1 , sizeof(unsigned char) , rFile);
			//fwrite (&g , 1 , sizeof(unsigned char) , gFile);
			//fwrite (&b , 1 , sizeof(unsigned char) , bFile);

			// Integer
			fprintf(rOrig, "%d\n", (int)r);
			fprintf(gOrig, "%d\n", (int)g);
			fprintf(bOrig, "%d\n", (int)b);
			fprintf(rgbIntFile, "(%d, %d):\tr: %d,\tg: %d,\tb: %d,\n", i, j, (int)r, (int)g, (int)b);

			// Record image
			image[i][j].b	= b;
			image[i][j].g	= g;
			image[i][j].r	= r;
		}
	}

	fclose(fptr);
	fclose(rgbIntFile);
	fclose(rOrig);
	fclose(gOrig);
	fclose(bOrig);

	if (proc == FALSE)
	{
		return;
	}

	// Operation function
	//add50(image, infoheader.width, infoheader.height);
	//emboss(image, infoheader.width, infoheader.height);
	//takeleft(image, infoheader.width, infoheader.height);
	//sharpen(image, infoheader.width, infoheader.height);
	//bayer(image, infoheader.width, infoheader.height);
	rgb2ycc(image, infoheader.width, infoheader.height);

	buf = (char *)malloc(sizeof(char) * header.offset);
	
	if ((fptr = fopen(inFilename,"r+b")) == NULL) {
		fprintf(stderr,"Unable to open BMP file \"%s\" the 2nd time\n",inFilename);
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
}

