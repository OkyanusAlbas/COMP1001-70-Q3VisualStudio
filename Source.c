#define _CRT_SECURE_NO_WARNINGS  // Disable deprecation warnings for fscanf and fopen

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Function declarations
void Gaussian_Blur(unsigned char* frame, unsigned char* filt, int M, int N);
void Sobel(unsigned char* filt, unsigned char* gradient, int M, int N);
int get_image_dimensions(const char* filename, int* M, int* N);
void read_image(const char* filename, unsigned char* frame, int M, int N);
void write_image(const char* filename, unsigned char* output_image, int M, int N);
void process_all_images(const char* input_dir, const char* output_dir);

// Constants
#define MAX_PATH_LENGTH 1024

int main() {
    const char* input_dir = "C:\\Users\\albao\\documents\\dev\\Question3\\input_images";
    const char* output_dir = "C:\\Users\\albao\\documents\\dev\\Question3\\output_images";

    // Process all images from a0.pgm to a30.pgm
    process_all_images(input_dir, output_dir);

    return 0;
}

void process_all_images(const char* input_dir, const char* output_dir) {
    char input_filename[MAX_PATH_LENGTH];
    char output_filename[MAX_PATH_LENGTH];
    unsigned char* frame;
    unsigned char* filt;
    unsigned char* gradient;
    int M, N;

    // Loop through image filenames from a0.pgm to a30.pgm
    for (int i = 0; i <= 30; i++) {
        // Construct the input file path
        snprintf(input_filename, MAX_PATH_LENGTH, "%s\\a%d.pgm", input_dir, i);

        // Get image dimensions dynamically
        if (get_image_dimensions(input_filename, &M, &N) != 0) {
            fprintf(stderr, "Error: Could not read dimensions of image %s\n", input_filename);
            continue;
        }

        // Dynamically allocate memory for the frame, filtered image, and gradient
        frame = (unsigned char*)malloc(M * N * sizeof(unsigned char));
        filt = (unsigned char*)malloc(M * N * sizeof(unsigned char));
        gradient = (unsigned char*)malloc(M * N * sizeof(unsigned char));

        // Check if memory allocation was successful
        if (frame == NULL || filt == NULL || gradient == NULL) {
            fprintf(stderr, "Memory allocation failed for image %s\n", input_filename);
            free(frame); free(filt); free(gradient);
            continue; // Skip to next image
        }

        // Read the image
        read_image(input_filename, frame, M, N);

        // Apply Gaussian Blur and Sobel edge detection
        Gaussian_Blur(frame, filt, M, N);
        Sobel(filt, gradient, M, N);

        // Construct the output file paths
        snprintf(output_filename, MAX_PATH_LENGTH, "%s\\blurred_a%d.pgm", output_dir, i);
        write_image(output_filename, filt, M, N);

        snprintf(output_filename, MAX_PATH_LENGTH, "%s\\edges_a%d.pgm", output_dir, i);
        write_image(output_filename, gradient, M, N);

        // Free dynamically allocated memory for the current image
        free(frame);
        free(filt);
        free(gradient);
    }
}

int get_image_dimensions(const char* filename, int* M, int* N) {
    FILE* finput = fopen(filename, "rb");
    if (finput == NULL) {
        perror("Error opening file");
        return -1;
    }

    char header[100];
    fscanf(finput, "%s", header); // Read PGM header

    // Read image dimensions
    if (fscanf(finput, "%d %d", M, N) != 2) {
        fprintf(stderr, "Error: Invalid image dimensions in file %s\n", filename);
        fclose(finput);
        return -1;
    }

    // Skip the max pixel value line
    int max_value;
    fscanf(finput, "%d", &max_value);

    fclose(finput);
    return 0;
}

void Gaussian_Blur(unsigned char* frame, unsigned char* filt, int M, int N) {
    const signed char Mask[5][5] = {
        {2,4,5,4,2},
        {4,9,12,9,4},
        {5,12,15,12,5},
        {4,9,12,9,4},
        {2,4,5,4,2}
    };
    int row, col, rowOffset, colOffset;
    int newPixel;
    unsigned char pix;
    const unsigned short int size = 2;

    // Apply Gaussian Blur
    for (row = 0; row < N; row++) {
        for (col = 0; col < M; col++) {
            newPixel = 0;
            for (rowOffset = -size; rowOffset <= size; rowOffset++) {
                for (colOffset = -size; colOffset <= size; colOffset++) {
                    if ((row + rowOffset < 0) || (row + rowOffset >= N) || (col + colOffset < 0) || (col + colOffset >= M))
                        pix = 0;
                    else
                        pix = frame[M * (row + rowOffset) + col + colOffset];
                    newPixel += pix * Mask[size + rowOffset][size + colOffset];
                }
            }
            filt[M * row + col] = (unsigned char)(newPixel / 159); // Normalize the value
        }
    }
}

void Sobel(unsigned char* filt, unsigned char* gradient, int M, int N) {
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
    int row, col, rowOffset, colOffset;
    int Gx, Gy;

    // Apply Sobel Edge Detection
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
            gradient[M * row + col] = (unsigned char)sqrt(Gx * Gx + Gy * Gy); // Calculate gradient strength
        }
    }
}

void read_image(const char* filename, unsigned char* frame, int M, int N) {
    FILE* finput = fopen(filename, "rb");
    if (finput == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char header[100];
    fscanf(finput, "%s", header); // Read PGM header

    // Skip image dimensions line
    int max_value;
    fscanf(finput, "%d %d", &M, &N); // Read image dimensions

    fscanf(finput, "%d", &max_value); // Skip max pixel value

    // Read image data
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            frame[M * i + j] = fgetc(finput);
        }
    }

    fclose(finput);
}

void write_image(const char* filename, unsigned char* output_image, int M, int N) {
    FILE* foutput = fopen(filename, "wb");
    if (foutput == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(foutput, "P2\n");
    fprintf(foutput, "%d %d\n", M, N);
    fprintf(foutput, "255\n");

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            fprintf(foutput, "%3d ", output_image[M * i + j]);
            if ((j + 1) % 32 == 0) fprintf(foutput, "\n");
        }
        fprintf(foutput, "\n");
    }

    fclose(foutput);
}
