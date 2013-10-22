
void conv_2d(int **kernel, int kernelSize, pixel **image, int width, int height, double factor, int bias);

// take the left pixel
void takeleft(pixel **image, int width, int height);

// Operation for test, add 50 to all rgb values
void add50(pixel **image, int width, int height);

void emboss(pixel **image, int width, int height);

void sharpen(pixel **image, int width, int height);
