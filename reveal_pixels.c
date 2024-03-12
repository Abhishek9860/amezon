#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <jpeglib.h>

void revealPixels(uint8_t* image, int width, int height, int revealNumber) {
    srand(revealNumber);

    for (int i = 0; i < revealNumber; ++i) {
        int x = rand() % width;
        int y = rand() % height;

        // Assuming 3 channels for RGB
        int index = (y * width + x) * 3;

        // Revealing the pixel (setting it to white)
        image[index] = 255; // Red
        image[index + 1] = 255; // Green
        image[index + 2] = 255; // Blue
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_image.jpg> <output_image.jpg> <reveal_number>\n", argv[0]);
        return 1;
    }

    const char* inputFileName = argv[1];
    const char* outputFileName = argv[2];
    int revealNumber = atoi(argv[3]);

    FILE* inputFile = fopen(inputFileName, "rb");
    if (!inputFile) {
        fprintf(stderr, "Error opening input file.\n");
        return 1;
    }

    // Initialize JPEG decompression structures
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Specify the input file
    jpeg_stdio_src(&cinfo, inputFile);

    // Read JPEG header
    jpeg_read_header(&cinfo, TRUE);

    // Start decompression
    jpeg_start_decompress(&cinfo);

    // Allocate memory for pixel data
    uint8_t* image = (uint8_t*)malloc(cinfo.output_width * cinfo.output_height * cinfo.output_components);

    // Read image data
    while (cinfo.output_scanline < cinfo.output_height) {
        uint8_t* row = &image[cinfo.output_scanline * cinfo.output_width * cinfo.output_components];
        jpeg_read_scanlines(&cinfo, &row, 1);
    }

    // Reveal pixels based on the input number
    revealPixels(image, cinfo.output_width, cinfo.output_height, revealNumber);

    // Clean up JPEG decompression structures
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(inputFile);

    // Save the modified image
    FILE* outputFile = fopen(outputFileName, "wb");
    if (!outputFile) {
        fprintf(stderr, "Error opening output file.\n");
        free(image);
        return 1;
    }

    // Initialize JPEG compression structures
    struct jpeg_compress_struct cinfo_compress;
    cinfo_compress.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo_compress);

    // Specify the output file
    jpeg_stdio_dest(&cinfo_compress, outputFile);

    // Set image parameters
    cinfo_compress.image_width = cinfo.output_width;
    cinfo_compress.image_height = cinfo.output_height;
    cinfo_compress.input_components = cinfo.output_components;
    cinfo_compress.in_color_space = cinfo.out_color_space;

    // Set default compression parameters
    jpeg_set_defaults(&cinfo_compress);

    // Start compression
    jpeg_start_compress(&cinfo_compress, TRUE);

    // Write image data
    while (cinfo_compress.next_scanline < cinfo_compress.image_height) {
        uint8_t* row = &image[cinfo_compress.next_scanline * cinfo_compress.image_width * cinfo_compress.input_components];
        jpeg_write_scanlines(&cinfo_compress, &row, 1);
    }

    // Finish compression
    jpeg_finish_compress(&cinfo_compress);

    // Clean up JPEG compression structures
    jpeg_destroy_compress(&cinfo_compress);

    fclose(outputFile);
    free(image);

    return 0;
}
