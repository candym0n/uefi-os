#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdint.h>
#include <stdio.h>
#include "format.h"
#include <stdbool.h>

// Format an image to this custom FS
bool format_image(FILE *image, int argc, char **argv);

// Copy a file to this FS and return the inode number of it
uint64_t copy_file(FILE *image, int argc, char **argv);

// Create a new directory in this FS and return the inode number of it
uint64_t create_dir(FILE *image, int argc, char **argv);

#endif  // OPTIONS_H
