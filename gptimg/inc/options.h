#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stdint.h>
#include "gpt.h"
#include "helpers.h"
#include "fat32.h"

// Get a command line argument (e.g command name value)
char *get_argument(int argc, char **argv, const char *name);

// Create an image and initialize it with an MBR and GPT Header
bool create_image(char *filename, int argc, char **argv);

// Add a partition to a GPT-formatted disk image
bool add_partition(FILE *image, int argc, char **argv);

// Format a partition to FAT32
bool format_partition(FILE *image, int argc, char **argv);

// Add a directory to a path
bool add_directory(FILE *image, int argc, char **argv);

// Add a file to a path given an existing one
bool add_file(FILE *image, int argc, char **argv);

#endif
