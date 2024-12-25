#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <emmintrin.h>
#include <limits.h>
#include <pmmintrin.h>
#include <immintrin.h>

// Function declarations
void Gaussian_Blur();
void Sobel();
void read_image(const char* filename);
void write_image2(const char* filename, unsigned char* output_image);
void openfile(const char* filename, FILE** finput);
int getint(FILE* fp);

// Global variable for header to store the file format
char header[100];  // This stores the image header (P2, P5, etc.)

// CRITICAL POINT: images' paths - Modify these paths as per your requirements
#define IN_PATH "C:\\Users\\albao\\documents\\dev\\Question3\\input_images\\"
#define OUT_PATH "C:\\Users\\albao\\documents\\dev\\Question3\\output_images\\"
#define OUT2_PATH "C:\\Users\\albao\\documents\\dev\\Question3\\output_images2\\"

// Number of images to process
#define NUM_IMAGES 31

// Image dimensions (These will be dynamically determined for each image)
int M, N;

// Dynamically allocated arrays for input and output images
unsigned char* frame1;  // input image
unsigned char* filt;    // output filtered image (Gaussian)
unsigned char* gradient; // output gradient image (Sobel)

// Gaussian mask (5x5)
const signed char Mask[5][5] = {
    {2,4,5,4,2},
    {4,9,12,9,4},
    {5,12,15,12,5},
    {4,9,12,9,4},
    {2,4,5,4,2}
};

// Sobel masks (3x3)
const signed char GxMask[3][3] = {
    {-1,0,1},
    {-2,0,2},
    {-1,0,1}
};

const signed char GyMask[3][3] = {
    {-1,-2,-1},
    {0,0,0},
    {1,2,1}
};

// Function to process all images
void process_all_images() {
    for (int i = 0; i < NUM_IMAGES; i++) {
        char input_filename[256], output_filename1[256], output_filename2[256];

        // Create filenames based on the image number (a0.pgm to a30.pgm)
        sprintf_s(input_filename, sizeof(input_filename), "%sa%d.pgm", IN_PATH, i);
        sprintf_s(output_filename1, sizeof(output_filename1), "%sblurred_a%d.pgm", OUT_PATH, i);
        sprintf_s(output_filename2, sizeof(output_filename2), "%sedge_detection_a%d.pgm", OUT2_PATH, i);

        // Read the image
        read_image(input_filename);

        // Apply Gaussian Blur and Sobel edge detection
        Gaussian_Blur();
        Sobel();

        // Write the processed images to the disk
        write_image2(output_filename1, filt);
        write_image2(output_filename2, gradient);

        // Free dynamically allocated memory after processing each image
        free(frame1);
        free(filt);
        free(gradient);
    }
}

int main() {
    // Process all images
    process_all_images();
    return 0;
}

void Gaussian_Blur() {
    int row, col, rowOffset, colOffset;
    int newPixel;
    unsigned char pix;
    const unsigned short int size = 2;

    for (row = 0; row < N; row++) {
        for (col = 0; col < M; col++) {
            newPixel = 0;
            for (rowOffset = -size; rowOffset <= size; rowOffset++) {
                for (colOffset = -size; colOffset <= size; colOffset++) {
                    if ((row + rowOffset < 0) || (row + rowOffset >= N) || (col + colOffset < 0) || (col + colOffset >= M))
                        pix = 0;
                    else
                        pix = frame1[M * (row + rowOffset) + col + colOffset];

                    newPixel += pix * Mask[size + rowOffset][size + colOffset];
                }
            }
            filt[M * row + col] = (unsigned char)(newPixel / 159);
        }
    }
}

void Sobel() {
    int row, col, rowOffset, colOffset;
    int Gx, Gy;

    for (row = 1; row < N - 1; row++) {
        for (col = 1; col < M - 1; col++) {
            Gx = 0;
            Gy = 0;

            for (rowOffset = -1; rowOffset <= 1; rowOffset++) {
                for (colOffset = -1; colOffset <= 1; colOffset++) {
                    Gx += filt[M * (row + rowOffset) + col + colOffset] * GxMask[rowOffset + 1][colOffset + 1];
                    Gy += filt[M * (row + rowOffset) + col + colOffset] * GyMask[rowOffset + 1][colOffset + 1];
                }
            }

            gradient[M * row + col] = (unsigned char)sqrt(Gx * Gx + Gy * Gy);
        }
    }
}

void read_image(const char* filename) {
    FILE* finput;
    int temp;

    // Open the image file
    openfile(filename, &finput);

    // Parse the header to get image dimensions (P2 or P5 format)
    if ((header[0] == 'P') && (header[1] == '5')) {
        M = getint(finput);  // Read width (M)
        N = getint(finput);  // Read height (N)
    }
    else if ((header[0] == 'P') && (header[1] == '2')) {
        M = getint(finput);  // Read width (M)
        N = getint(finput);  // Read height (N)
    }
    else {
        printf("\nError: Unsupported image format.");
        exit(EXIT_FAILURE);
    }

    // Dynamically allocate memory for the image
    frame1 = (unsigned char*)malloc(M * N * sizeof(unsigned char));
    filt = (unsigned char*)malloc(M * N * sizeof(unsigned char));
    gradient = (unsigned char*)malloc(M * N * sizeof(unsigned char));

    // Read the image data
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < M; i++) {
            temp = getc(finput);
            frame1[M * j + i] = (unsigned char)temp;
        }
    }

    fclose(finput);
    printf("\nImage successfully read from disk: %s\n", filename);
}

void write_image2(const char* filename, unsigned char* output_image) {
    FILE* foutput;

    printf("Writing result to disk...\n");

    // Open output file
    if (fopen_s(&foutput, filename, "wb") != 0) {
        fprintf(stderr, "Unable to open file %s for writing\n", filename);
        exit(-1);
    }

    // Write the PGM header
    fprintf(foutput, "P2\n");
    fprintf(foutput, "%d %d\n", M, N);
    fprintf(foutput, "255\n");

    // Write the pixel data
    for (int j = 0; j < N; ++j) {
        for (int i = 0; i < M; ++i) {
            fprintf(foutput, "%3d ", output_image[M * j + i]);
            if (i % 32 == 31) fprintf(foutput, "\n");
        }
        if (M % 32 != 0) fprintf(foutput, "\n");
    }

    fclose(foutput);
    printf("Output written to %s\n", filename);
}

void openfile(const char* filename, FILE** finput) {
    int x0, y0, x, aa;

    if ((fopen_s(finput, filename, "rb")) != 0) {
        fprintf(stderr, "Unable to open file %s for reading\n", filename);
        exit(-1);
    }

    aa = fscanf_s(*finput, "%s", header, 20);

    x0 = getint(*finput); // This is M (width)
    y0 = getint(*finput); // This is N (height)
    printf("\tHeader is %s, M=%d, N=%d\n", header, x0, y0);

    x = getint(*finput); // Read and discard the max pixel value
}

int getint(FILE* fp) {
    int c, i;

    c = getc(fp);
    while (c == ' ' || c == '\n' || c == '\t') {
        c = getc(fp);
    }

    i = 0;
    while (c >= '0' && c <= '9') {
        i = (i * 10) + (c - '0');
        c = getc(fp);
    }

    return i;
}
