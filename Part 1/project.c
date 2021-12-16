#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bmp.h"

#define default_kernel_size 5 

/** Function to load a BMP image from disk
Parameters:
	filename, pointer to the data structure to load the image into
Outputs:
	0 if loading was successful, a number !=0 otherwise
*/
int loadBMP(char *filename, BMP_Image *image) {
	
	FILE *fp;

	fp = fopen(filename, "rb");
	if(fp == NULL)
	{
		printf("Error: impossibile aprire il file in lettura\n");
		return 1;
	}

	fread(image->magic, sizeof(image->magic), 1, fp );

	if(image->magic[0]!='B' || image->magic[1]!='M')
	{
		printf("Error: tipo di immagine non corretto\n");
		return 2;
	}

	fread(&image->header, sizeof(image->header), 1, fp);
	fread(&image->info, sizeof(image->info), 1, fp);

	if(image->info.bits!=8)
	{
		printf("Error: numero di bits/pixel diverso da 8\n");
		return 3;
	}

	if(image->info.width!=DATA_DIM || image->info.height!=DATA_DIM)
	{
		printf("--- Attenzione, dimensioni non corrette ---");
	}

	fread(&image->color_table, sizeof(image->color_table), 1, fp);
	fread(image->data, sizeof(image->data), 1, fp);

	fclose(fp);
	return 0;

}


/** Function to save a BMP_Image structure on disk
Parameters:
	pointer to the data structure to save, filename
Outputs:
	0 if saving was successful, 1 otherwise
*/
int saveBMP(BMP_Image image, char * filename) {
	
	FILE *fp2;
	fp2 = fopen(filename, "wb");

	if(fp2==NULL)
	{
		printf("Impossibile aprire il file in scrittura\n");
		return 1;
	}

	fwrite(&image.magic, sizeof(image.magic), 1, fp2);
	fwrite(&image.header, sizeof(image.header), 1, fp2);
	fwrite(&image.info, sizeof(image.info), 1, fp2);
	fwrite(&image.color_table, sizeof(image.color_table), 1, fp2);
	fwrite(image.data, sizeof(image.data), 1, fp2);

	fclose(fp2);
	return 0;

}


/** Function to dynamically allocate the gaussian kernel using the gaussian filter equation
Parameters: 
	kernel size n
Output:
	pointer to the matrix generated
*/
double **create_kernel(int n) {


	// Dynamic allocation of a n*n matrix

	int i,j;
	double **kernel = (double **)malloc(n * sizeof(double *));
	for(i = 0; i < n; i++)
		kernel[i] = (double *)malloc(n * sizeof(double));


	// Populating the matrix
	// gaussian filter equation: (e^(-(x^2+y^2)/(2*sigma^2)))/(2*pi*sigma^2)

	double mean = n/2, sum = 0.0, sigma = 1;
	for(i = 0; i < n; i++) {
		for(j = 0; j < n; j++) {
			kernel[i][j] = exp(-0.5 * (pow((i-mean)/sigma,2)+pow((j-mean)/sigma,2))) / (2*M_PI*sigma*sigma);
			sum += kernel[i][j];
		}
	}


	// Normalisation

	for(i = 0; i < n; i++) {
		for(j = 0; j < n; j++) {
			kernel[i][j] /= sum;
			//printf("%f ", kernel[i][j]);
		}
		//printf("\n");
	}


	// Output		
	return kernel;
}


/** Function to free the heap space occupied by the gauss kernel
Parameters: 
	pointer to the kernel, size of the kernel
*/
void free_kernel(double **kernel, int n) {
	for(int i = 0; i < n; i++)
		free(kernel[i]);
	free(kernel);
}

/** Function to apply the gaussian filter to the input image and save the result
Parameters: 
	input image pathname, kernel size
*/
void gaussian_filter(char *pathname, int n) {


	// Loading BMP images (input and output)

	BMP_Image image, gauss_output;
	int e;
	e = loadBMP(pathname, &image);
	e = loadBMP(pathname, &gauss_output);
	

	// Kernel creation

	double **kernel = create_kernel(n);


	// Gauss filter implementation

	int i,j,r,s,sum;

	for (i = 0; i < DATA_DIM; i++) {
		for (j = 0; j < DATA_DIM; j++) {
		
			sum = 0;
			for (r = 0; r < n; r++) {
				for (s = 0; s < n; s++) {
					if (i+r-(n/2) >= 0 && i+r-(n/2) < DATA_DIM && j+s-(n/2) >= 0 && j+s-(n/2) < DATA_DIM)
						sum += ((double)image.data[i+r-(n/2)][j+s-(n/2)].grey)*kernel[r][s];
				}	
			}
			
			if(n > 0)
				gauss_output.data[i][j].grey = (unsigned char)sum;
				
		}
	}
	

	// Saving output and freeing heap space

	e = saveBMP(gauss_output, "gauss_output.bmp");
	free_kernel(kernel, n);  
	
}


/** Function to apply the Sobel filter to the input image and save the result
Parameters: 
	input image pathname
*/
void sobel_filter(char *pathname) {


	// Loading BMP images (input and output)

	BMP_Image image, sobel_output;
	int e;
	e = loadBMP(pathname, &image);
	e = loadBMP(pathname, &sobel_output);
	

	// Filters initialisation (horizontal & vertical)

	int h_filter[3][3] = {{ -1, 0, 1 },
           		     { -2, 0, 2 },
	  		     { -1, 0, 1 }};
	int v_filter[3][3] = {{ -1, -2, -1 },
	     		     {  0,  0,  0 },
			     {  1,  2,  1 }};
	

	// Sobel filter implementation

	int i,j,r,s;
	double h_gradient, v_gradient, magnitude;

	for (i = 0; i < DATA_DIM; i++) {
		for (j = 0; j < DATA_DIM; j++) {
		
			h_gradient = 0;
			v_gradient = 0;
			for (r = 0; r < 3; r++) {
				for (s = 0; s < 3; s++) {
				
					if (i+r-1 >= 0 && i+r-1 < DATA_DIM && j+s-1 >= 0 && j+s-1 < DATA_DIM) {
						h_gradient += ((double)image.data[i+r-1][j+s-1].grey) * h_filter[r][s];
						v_gradient += ((double)image.data[i+r-1][j+s-1].grey) * v_filter[r][s];
					}
					
				}
			}
			
			magnitude = sqrt( pow(h_gradient, 2) + pow(v_gradient, 2) );
			sobel_output.data[i][j].grey = (unsigned char)magnitude;
			
		}
	}
	

	// Saving output

	e = saveBMP(sobel_output, "sobel_output.bmp");
	
}



int main(int argc, char *argv[]) {
	
	// This program applies separately a gaussian filter and a sobel filter to a 256x256 bmp image
	
	// It takes as input the pathname of the image as argv[1] and an optional kernel size for the
	// gaussian filter as argv[2], which defaults to 5
	
	// The outputs are 'gauss_output.bmp' and 'sobel_output.bmp', both computed from the input image
	// To apply the sobel filter after the gauss filter, it's enough to run the program a second time
	// with pathname = 'gauss_output.bmp'

	// The file 'bmp.h' includes functions and data structures for BMP images


	// Input parsing

	char pathname[50];
	strcpy(pathname,argv[1]);
	
	int kernel_size;
	if (argc == 3)
		kernel_size = atoi(argv[2]);
	else
		kernel_size = default_kernel_size;
	

	// Filters functions
	
	gaussian_filter(pathname, kernel_size);
	sobel_filter(pathname);
	
	return 0;
}
