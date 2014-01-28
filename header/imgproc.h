
void dump_pixel(pixel **image, int width, int height, FILE *a_out, FILE *b_out, FILE *c_out, int dumpRGB);

void rgb2ycc (pixel **image, int width, int height);

void ycclut (pixel **image, int width, int height);

void ycc2rgb (pixel **image, int width, int height);

void conv_2d(int **kernel, int kernelSize, pixel **image, int width, int height, double factor, int bias);

// take the left pixel
void takeleft(pixel **image, int width, int height);

// Operation for test, add 50 to all rgb values
void add50(pixel **image, int width, int height);

void emboss(pixel **image, int width, int height);

void sharpen(pixel **image, int width, int height);

void bayer(pixel **image, int width, int height);

void demosaic_neighbor(pixel **image, int width, int height);

void demosaic_acpi(pixel **image, int width, int height);


