#include <stdio.h>
#include <stdlib.h>
#include <common/string.h>
#include "options.h"

// Commands
#define COMMAND_FORMAT "format"
#define COMMAND_COPY_FILE "cp"
#define COMMAND_ADD_DIR "mkdir"

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
        printf("Usage: fsformat <command> <file> <arguments>\n");
        return EXIT_FAILURE;
    }

    // The first two arguments are the command and file respectively
    char *command = argv[1];
    char *filename = argv[2];

    // Open the file
    FILE *image = fopen(filename, "rb+");
    if (image == NULL)
    {
        printf("Failed to open file %s!\n", filename);
        return EXIT_FAILURE;
    }

    // Check for commands
    return strcmp(command, COMMAND_FORMAT) == 0 ? execute_command(format_image(image, argc, argv), "Failed to format image!") :
    strcmp(command, COMMAND_ADD_DIR) == 0 ? execute_command(create_dir(image, argc, argv), "Failed to add directory!") :
    strcmp(command, COMMAND_COPY_FILE) == 0 ? execute_command(copy_file(image, argc, argv), "Failed to copy file!") :
    execute_command(false, "Unknown command!");
}
