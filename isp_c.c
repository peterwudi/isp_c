#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "string.h"

#include "header/rgb_basic.h"
#include "header/bmpToRGB.h"
#include "header/imgproc.h"


void main(int argc,char **argv)
{
	char buf[50] = {0}; 

	// Check arguments
	if (argc != 2) {
		fprintf(stderr,"Usage: %s inFilename\n",argv[0]);
		exit(-1);
	}

	bmpToRGB(argv[1], TRUE);

	printf("Press enter to exit...\n");
	getchar("%c\n");


}

