#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpt.h"
#include "helpers.h"
#include "options.h"
#include "config.h"

// Commands
#define CMD_CREATE_IMAGE "create"         // Create a disk image
#define CMD_ADD_PARTITION "add-partition" // Add a partition to a disk image

// Initialize lba_size (not in config.c, believe it or not)
uint32_t lba_size = 512;

// Execute a function (with error handling)
static inline int execute_command(bool result, char *message)
{
    if (result)
        return EXIT_SUCCESS;
    else
    {
        printf("ERROR: %s\n", message);
        return EXIT_FAILURE;
    }
}

int main(int argc, char **argv)
{
    // Check that we have enough arguments
    if (argc < 2)
    {
        printf("Usage: gptimg <command> <file> <arguments>\n");
        return EXIT_FAILURE;
    }

    // The first two arguments are the command and file respectively
    char *command = argv[1];
    char *filename = argv[2];

    // Create the CRC32 table
    create_crc32_table();

    // Set the LBA size
    char *lba_size_args_str = get_argument(argc, argv, "--lba-size");
    uint32_t lba_size_args;
    if (lba_size_args_str != NULL)
    {
        lba_size_args = strtol(lba_size_args_str, NULL, 10);

        if (lba_size_args != 512 || lba_size_args != 4096)
            printf("LBA size MUST be either 512 or 4096, not %u. Defaulting to %u.\n", lba_size_args, lba_size);
        else
            lba_size = lba_size_args;
    }

    // Check if we want to create an image
    if (strcmp(command, CMD_CREATE_IMAGE) == 0)
        return execute_command(create_image(filename, argc, argv), "Failed to create image!");

    // Open the file
    FILE *image = fopen(filename, "rb+");
    if (image == NULL)
    {
        printf("Failed to open file %s!\n", filename);
        return EXIT_FAILURE;
    }

    // Check for other commands
    return strcmp(command, CMD_ADD_PARTITION) == 0 ? execute_command(add_partition(image, argc, argv), "Failed to add partition!") :
        printf("Invalid command %s!\n", command) & 0 + EXIT_FAILURE;
}
