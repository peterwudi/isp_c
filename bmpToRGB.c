#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif


/*
	Read a BMP file
	Write out RGB, and binary form
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

int main(int argc,char **argv)
{
	int				i,j					= 0;
	int				gotindex			= FALSE;
	unsigned char	r, g, b, pad		= '\0';
	int				padSize				= 0;
	char			buf[50]				= {0};
	HEADER			header				= {0};
	INFOHEADER		infoheader			= {0};
	COLOURINDEX		colourindex[256]	= {0};
	
	FILE			*fptr		= NULL;
	FILE			*rgbFile	= NULL;
	FILE			*rFile		= NULL;
	FILE			*gFile		= NULL;
	FILE			*bFile		= NULL;
	FILE			*rgbIntFile	= NULL;
	FILE			*rIntFile	= NULL;
	FILE			*gIntFile	= NULL;
	FILE			*bIntFile	= NULL;

	// Check arguments
	if (argc != 2) {
		fprintf(stderr,"Usage: %s inFilename\n",argv[0]);
		exit(-1);
	}

	// Open file
	if ((fptr = fopen(argv[1],"rb")) == NULL) {
		fprintf(stderr,"Unable to open BMP file \"%s\"\n",argv[1]);
		exit(-1);
	}

	if ((rgbFile = fopen("rgb","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"rgb\"\n");
		exit(-1);
	}
	if ((rFile = fopen("r","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"r\"\n");
		exit(-1);
	}
	if ((gFile = fopen("g","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"g\"\n");
		exit(-1);
	}
	if ((bFile = fopen("b","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"b\"\n");
		exit(-1);
	}
	if ((rgbIntFile = fopen("rgbIntFile","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"rgbIntFile\"\n");
		exit(-1);
	}
	if ((rIntFile = fopen("rIntFile","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"rIntFile\"\n");
		exit(-1);
	}
	if ((gIntFile = fopen("gIntFile","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"gIntFile\"\n");
		exit(-1);
	}
	if ((bIntFile = fopen("bIntFile","wb")) == NULL) {
		fprintf(stderr,"Unable to open target file \"bIntFile\"\n");
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
			fwrite (&r, 1 , sizeof(unsigned char) , rgbFile);
			fwrite (&g, 1 , sizeof(unsigned char) , rgbFile);
			fwrite (&b, 1 , sizeof(unsigned char) , rgbFile);

			fwrite (&r , 1 , sizeof(unsigned char) , rFile);
			fwrite (&g , 1 , sizeof(unsigned char) , gFile);
			fwrite (&b , 1 , sizeof(unsigned char) , bFile);

			// Integer
			fprintf(rIntFile, "%d\n", (int)r);
			fprintf(gIntFile, "%d\n", (int)g);
			fprintf(bIntFile, "%d\n", (int)b);
			fprintf(rgbIntFile, "(%d, %d):\tr: %d,\tg: %d,\tb: %d,\n", i, j, (int)r, (int)g, (int)b);
		}
	}

	fclose(fptr);
	fclose(rgbFile);
	fclose(rFile);
	fclose(gFile);
	fclose(bFile);
	fclose(rgbIntFile);
	fclose(rIntFile);
	fclose(gIntFile);
	fclose(bIntFile);

	printf("Press enter to exit...\n");
	scanf("%c", buf);
}

