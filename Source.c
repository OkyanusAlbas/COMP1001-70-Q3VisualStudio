#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define IN "input_images/a%d.pgm"  // Path pattern for input images
#define OUT "output_images/a%d_gaussian.pgm"  // Output for Gaussian Blur images
#define OUT2 "output_images/a%d_sobel.pgm"  // Output for Sobel edge detection images

// Function prototypes
int get_image_dimensions(FILE* file, int* width, int* height, int* max_val, int* is_binary);
unsigned char* read_image(FILE* file, int width, int height, int is_binary);
void write_image(const char* filename, unsigned char* image, int width, int height, int max_val);
void gaussian_blur(unsigned char* input, unsigned char* output, int width, int height);
void sobel_edge_detection(unsigned char* input, unsigned char* output, int width, int height);

int main() {
    int i, width, height, max_val, is_binary;
    FILE* input_file;
    unsigned char* image, * gaussian_image, * sobel_image;

    // Loop through all 31 input images
    for (i = 0; i < 31; i++) {
        // Construct input filename dynamically based on index
        char input_filename[50];
        sprintf_s(input_filename, sizeof(input_filename), IN, i);

        // Open the image file
        errno_t err = fopen_s(&input_file, input_filename, "r");
        if (err != 0) {
            printf("Error opening file: %s\n", input_filename);
            continue;  // Skip if the image can't be opened
        }

        // Read image dimensions and header
        if (!get_image_dimensions(input_file, &width, &height, &max_val, &is_binary)) {
            printf("Failed to read image: %s\n", input_filename);
            fclose(input_file);
            continue;  // Skip if dimensions cannot be read
        }

        // Read image data
        image = read_image(input_file, width, height, is_binary);
        if (!image) {
            printf("Failed to read image data: %s\n", input_filename);
            fclose(input_file);
            continue;  // Skip if data can't be read
        }

        // Allocate memory for output images based on dimensions of the current image
        gaussian_image = (unsigned char*)malloc(width * height * sizeof(unsigned char));
        sobel_image = (unsigned char*)malloc(width * height * sizeof(unsigned char));

        if (!gaussian_image || !sobel_image) {
            printf("Memory allocation failed.\n");
            free(image);
            fclose(input_file);
            continue;  // Skip if memory allocation fails
        }

        // Apply Gaussian Blur filter to the image
        gaussian_blur(image, gaussian_image, width, height);

        // Apply Sobel edge detection filter to the image
        sobel_edge_detection(image, sobel_image, width, height);

        // Construct output filenames dynamically based on image index
        char output_filename[50];

        // Write Gaussian Blur image to file
        sprintf_s(output_filename, sizeof(output_filename), OUT, i);
        write_image(output_filename, gaussian_image, width, height, max_val);

        // Write Sobel edge detection image to file
        sprintf_s(output_filename, sizeof(output_filename), OUT2, i);
        write_image(output_filename, sobel_image, width, height, max_val);

        // Free allocated memory
        free(image);
        free(gaussian_image);
        free(sobel_image);
        fclose(input_file);
    }

    return 0;
}

// Function to get image dimensions from PGM header (handles both P2 and P5 formats)
int get_image_dimensions(FILE* file, int* width, int* height, int* max_val, int* is_binary) {
    char header[3];

    // Read the file header
    if (fscanf_s(file, "%2s", header, (unsigned int)sizeof(header)) != 1) {
        printf("Error: Failed to read header.\n");
        return 0;  // Error reading the header
    }

    // Null-terminate the string to avoid issues
    header[2] = '\0';

    // Debugging output for the header
    printf("Header: %s\n", header);

    // Check if the header matches expected PGM formats
    if (strcmp(header, "P2") == 0) {
        *is_binary = 0;  // ASCII format
        printf("Format: P2 (ASCII)\n");
    }
    else if (strcmp(header, "P5") == 0) {
        *is_binary = 1;  // Binary format
        printf("Format: P5 (Binary)\n");
    }
    else {
        printf("Error: Not a valid PGM file (Header: %s)\n", header);
        return 0;  // Invalid header
    }

    // Read image dimensions and max pixel value
    if (fscanf_s(file, "%d %d", width, height) != 2) {
        printf("Error: Failed to read image dimensions. File might be malformed.\n");
        return 0;  // Error reading dimensions
    }

    if (fscanf_s(file, "%d", max_val) != 1) {
        printf("Error: Failed to read max pixel value.\n");
        return 0;  // Error reading max pixel value
    }

    // Debugging output for dimensions and max value
    printf("Dimensions: %d x %d, Max value: %d\n", *width, *height, *max_val);

    // Skip over any extra newline characters that might be present
    while (fgetc(file) != '\n');

    return 1;  // Successfully read dimensions and max value
}

// Function to read image data from PGM file (handles both P2 and P5)
unsigned char* read_image(FILE* file, int width, int height, int is_binary) {
    unsigned char* image = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    if (!image) {
        return NULL;  // Memory allocation failed
    }

    if (is_binary) {
        // Read binary PGM (P5 format)
        fread(image, sizeof(unsigned char), width * height, file);
    }
    else {
        // Read ASCII PGM (P2 format)
        for (int i = 0; i < width * height; i++) {
            if (fscanf_s(file, "%hhu", &image[i]) != 1) {
                free(image);
                return NULL;  // Reading failed
            }
        }
    }

    return image;
}

// Function to write image data to file
void write_image(const char* filename, unsigned char* image, int width, int height, int max_val) {
    FILE* file;
    errno_t err = fopen_s(&file, filename, "w");
    if (err != 0) {
        printf("Error opening file for writing: %s\n", filename);
        return;
    }

    // Write PGM header
    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "%d\n", max_val);

    // Write pixel values
    for (int i = 0; i < width * height; i++) {
        fprintf(file, "%d ", image[i]);
        if ((i + 1) % width == 0) {
            fprintf(file, "\n");
        }
    }

    fclose(file);
}

// Function to apply Gaussian Blur filter to an image
void gaussian_blur(unsigned char* input, unsigned char* output, int width, int height) {
    // Gaussian kernel (simplified version)
    int kernel[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };

    int kernel_sum = 16;  // Sum of all values in the kernel

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sum = 0;

            // Apply kernel to each pixel
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel = input[(y + ky) * width + (x + kx)];
                    sum += pixel * kernel[ky + 1][kx + 1];
                }
            }

            // Normalize the result
            output[y * width + x] = sum / kernel_sum;
        }
    }
}

// Function to apply Sobel edge detection filter to an image
void sobel_edge_detection(unsigned char* input, unsigned char* output, int width, int height) {
    int gx, gy;
    int sobel_x[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int sobel_y[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            gx = 0;
            gy = 0;

            // Apply Sobel kernel to each pixel
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel = input[(y + ky) * width + (x + kx)];
                    gx += pixel * sobel_x[ky + 1][kx + 1];
                    gy += pixel * sobel_y[ky + 1][kx + 1];
                }
            }

            // Combine the gradients and store the result
            int magnitude = (int)sqrt(gx * gx + gy * gy);
            output[y * width + x] = (unsigned char)(magnitude > 255 ? 255 : magnitude);
        }
    }
}
