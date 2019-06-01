#include <stdio.h>
#include <unistd.h>

/* This program visualises convolution filters
 */

void load_tensor_with_scanf(char*** tensor, int* size)
{
	// Get tensor size
	printf("Enter desired dimensions: ");
	int x, y;
	scanf("%d %d", &x, &y);
	size[0] = x;
	size[1] = y;
	// Allocate tensor memory
	*tensor = calloc(x, sizeof(char*));
	for (int i=0; i < x; i++) (*tensor)[i] = calloc(y, sizeof(char));
	// Get tensor values
	for (int j=0; j < y; j++) for (int i=0; i < x; i++)
	{
		printf("Enter value for position (%2d,%2d): ", i, j);
		scanf("%d", &((*tensor)[i][j]));
	}
}

void apply_tensor_to_grid(char** grid, char** tensor, int* size, int x, int y, int* highlight)
{
	for (int r=0; r < size[1]; r++) for (int c=0; c < size[0]; c++)
	{
		int ypos = r   + y; // yposition is same as row
		int xpos = c*3 + x; // xposition is col * (2 (digits) + 1 (padding))
		char numberStr[20]; // 20 chars good enough for 64-bit integers
		for (int i=0; i < 20; i++) numberStr[i] = ' ';
		sprintf(numberStr, "%d", tensor[c][r]);
		int addBitToDigit = 0;
		if (highlight != 0) if (c >= highlight[0] && r >= highlight[1] &&
		  c < highlight[2] && r < highlight[3]) {
			// This bit is not used by any ASCII digits and will
			// indicate that a digit should be highlighted.
			addBitToDigit = 0x40;
		}
		for (int i=0; i < 3; i++) {
			if (numberStr[i] == 0) numberStr[i] = ' ';
			grid[ypos][xpos+i] = numberStr[i] | addBitToDigit;
		}
	}
}

void show_filter(
	char** filter, int* filterSize, int xpos)
{
	// Allocate grid
	int gWidth = 3*filterSize[0] + 3*xpos;
	int gHeight = filterSize[1]; // source is always highest
	char** grid = calloc(gHeight, sizeof(char*));
	for (int i=0; i < gHeight; i++) grid[i] = calloc(gWidth, sizeof(char*));
	// Draw filter tensor on grid
	apply_tensor_to_grid(grid, filter, filterSize, 3*xpos, 0, 0);
	// Draw the grid
	for (int r=0; r < gHeight; r++) { printf("   "); for (int c=0; c < gWidth; c++)
	{
		char cc = grid[r][c];
		if (cc & 0x40) {
			printf("\033[32;1m");
			cc = cc & 0xBF;
		}
		if (cc == 0) printf(" ");
		else printf("%c", cc);
		printf("\033[0m");
	} printf("\n"); }
}


void visualise_conv_step(
	char** source, int* sourceSize, char** filter, int* filterSize, char** output, int* outputSize,
	int* hlSource, int* hlOutput)
{

	int gap = 2;

	// Allocate grid
	int gWidth = 3*sourceSize[0] + 3*outputSize[1] + gap + 1;
	int gHeight = sourceSize[1]; // source is always highest
	int xOutput = 3*sourceSize[0] + gap; // position of output (x coord)
	char** grid = calloc(gHeight, sizeof(char*));
	for (int i=0; i < gHeight; i++) grid[i] = calloc(gWidth, sizeof(char*));
	// Draw source tensor on grid
	apply_tensor_to_grid(grid, source, sourceSize, 0, 0, hlSource);
	// Draw output tensor on grid
	apply_tensor_to_grid(grid, output, outputSize, xOutput, 0, hlOutput);
	// Draw the grid
	for (int r=0; r < gHeight; r++) { printf("   "); for (int c=0; c < gWidth; c++)
	{
		char cc = grid[r][c];
		if (cc & 0x40) {
			printf("\033[32;1m");
			cc = cc & 0xBF;
		}
		if (cc == 0) printf(" ");
		else printf("%c", cc);
		printf("\033[0m");
	} printf("\n"); }
	printf("\n\n");
}

void visualise_convolution(
	char** source, int* sourceSize, char** filter, int* filterSize, char** output, int* outputSize,
	int strideX, int strideY)
{
	for (int ox=0; ox < outputSize[0]; ox++)
	for (int oy=0; oy < outputSize[1]; oy++)
		output[ox][oy] = 0;

	// Note: ASSUME valid padding (which is a dumb name for it but I digress)
	for (int ox=0; ox < outputSize[0]; ox++)
	for (int oy=0; oy < outputSize[1]; oy++)
	{
		int sumThing = 0;

		int filterStartX = strideX*ox;
		int filterStartY = strideY*oy;
		for (int fx=0; fx < filterSize[0]; fx++)
		for (int fy=0; fy < filterSize[1]; fy++)
		{
			int v = filter[fx][fy] * source[fx + ox*strideX][fy + oy*strideY];
			printf("%d\n", v);
			sumThing += v;
		}
		output[ox][oy] = sumThing;


		// This array will specify part of grid to highlight (xBegin,yBegin,xEnd,yEnd)
		int hlSource[4];
		hlSource[0] = filterStartX;
		hlSource[1] = filterStartY;
		hlSource[2] = filterStartX + filterSize[0];
		hlSource[3] = filterStartY + filterSize[1];

		int hlOutput[4];
		hlOutput[0] = ox;
		hlOutput[1] = oy;
		hlOutput[2] = ox+1;
		hlOutput[3] = oy+1;

		for (int i = 0; i < 80; i++) printf("\n");

		show_filter(filter, filterSize, filterStartX);
		printf("\n\n");

		// This function will assume 1-2 digit values,
		// and that the viewport width is equal or greater
		// than the source width + filter width + 2.
		visualise_conv_step(
			source, sourceSize,
			filter, filterSize,
			output, outputSize,
			hlSource, hlOutput);
		
		char buffer[1];
		while (getchar() != 'a');
	}

}

int main()
{
	char** source;
	int* sourceSize;
	char** filter;
	int* filterSize;
	char** output;
	int* outputSize;

	sourceSize = calloc(6, sizeof(int));
	filterSize = sourceSize + 2; // memory location, not value
	outputSize = sourceSize + 4; // memory location, not value
	load_tensor_with_scanf(&source, sourceSize);
	load_tensor_with_scanf(&filter, filterSize);
	
	outputSize[0] = sourceSize[0] - filterSize[0] + 1;
	outputSize[1] = sourceSize[1] - filterSize[1] + 1;

	output = calloc(outputSize[0], sizeof(char*));
	for (int ox=0; ox < outputSize[0]; ox++)
		output[ox] = calloc(outputSize[1], sizeof(char));

	// load_tensor_with_scanf(filter);
	visualise_convolution(source, sourceSize, filter, filterSize, output, outputSize, 1, 1);
}
