#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024
#define NUM_IMAGES 31  // From a0.pgm to a30.pgm

// Function to read PGM images
int read_image(const char* filename, unsigned char** image, int* width, int* height, int* max_value, int* is_binary) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Unable to open file %s for reading.\n", filename);
        return -1;
    }

    char magic_number[3];
    // Read the magic number (P2 or P5)
    fscanf_s(file, "%2s", magic_number, (unsigned int)sizeof(magic_number));  // Use fscanf_s
    if (magic_number[0] != 'P') {
        printf("Error: Invalid PGM file format. Expected 'P2' or 'P5'.\n");
        fclose(file);
        return -1;
    }

    *is_binary = (magic_number[1] == '5');  // Check if it's P5 (binary) or P2 (ASCII)

    // Skip comments in the header (if any)
    char c;
    do {
        c = fgetc(file);
    } while (c == '#');
    ungetc(c, file);  // Push the last character back for further reading

    // Read image dimensions and max pixel value
    fscanf_s(file, "%d", width);
    fscanf_s(file, "%d", height);
    fscanf_s(file, "%d", max_value);

    // Allocate memory for the image
    int image_size = (*width) * (*height);
    *image = (unsigned char*)malloc(image_size * sizeof(unsigned char));
    if (*image == NULL) {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        return -1;
    }

    // Read pixel data based on image format (binary or ASCII)
    if (*is_binary) {
        // For P5 (binary), read raw pixel data
        size_t data_size = (*width) * (*height);
        if (fread(*image, sizeof(unsigned char), data_size, file) != data_size) {
            printf("Error: Failed to read pixel data for P5 format.\n");
            free(*image);
            fclose(file);
            return -1;
        }
    }
    else {
        // For P2 (ASCII), read pixel data as ASCII values
        for (int i = 0; i < (*width) * (*height); i++) {
            fscanf_s(file, "%hhu", &(*image)[i]);
        }
    }

    fclose(file);
    return 0;
}

// Function to write PGM images (in both P2 and P5 formats)
int write_image(const char* filename, unsigned char* image, int width, int height, int max_value, int is_binary) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Unable to open file %s for writing.\n", filename);
        return -1;
    }

    if (is_binary) {
        // For P5 (binary format)
        fprintf(file, "P5\n");
        fprintf(file, "%d %d\n", width, height);
        fprintf(file, "%d\n", max_value);
        size_t data_size = width * height;
        if (fwrite(image, sizeof(unsigned char), data_size, file) != data_size) {
            printf("Error: Failed to write pixel data for P5 format.\n");
            fclose(file);
            return -1;
        }
    }
    else {
        // For P2 (ASCII format)
        fprintf(file, "P2\n");
        fprintf(file, "%d %d\n", width, height);
        fprintf(file, "%d\n", max_value);
        for (int i = 0; i < width * height; i++) {
            fprintf(file, "%d ", image[i]);
            if ((i + 1) % width == 0) {
                fprintf(file, "\n");
            }
        }
    }

    fclose(file);
    return 0;
}

int main() {
    unsigned char* image = NULL;
    int width, height, max_value, is_binary;

    // Loop through images from a0.pgm to a30.pgm
    for (int i = 0; i < NUM_IMAGES; i++) {
        // Construct input and output filenames
        char input_filename[MAX_BUFFER_SIZE];
        char output_filename[MAX_BUFFER_SIZE];

        // Create the input and output filenames (a0.pgm, a0_output.pgm, etc.)
        sprintf_s(input_filename, sizeof(input_filename), "input_images/a%d.pgm", i);  // Use sprintf_s
        sprintf_s(output_filename, sizeof(output_filename), "output_images/a%d_output.pgm", i);  // Use sprintf_s

        // Read the image
        if (read_image(input_filename, &image, &width, &height, &max_value, &is_binary) != 0) {
            printf("Error: Failed to read image %s.\n", input_filename);
            continue;  // Skip this image and continue with the next
        }

        // Output the image to a new file
        if (write_image(output_filename, image, width, height, max_value, is_binary) != 0) {
            printf("Error: Failed to write image %s.\n", output_filename);
            free(image);
            continue;  // Skip this image and continue with the next
        }

        // Clean up for the current image
        free(image);
        printf("Image %s processed and saved as %s\n", input_filename, output_filename);
    }

    return 0;
}
