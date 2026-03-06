#include <common/string.h>
#include "options.h"

// i is the index in argv of the argument name
static char *parse_value(char **argv, int i) {
    int j = 0;
    void *result = malloc(72);

    if (argv[i][0] == '"') {
        do {
            ((char *)result)[j++] = argv[i][j];
        } while (argv[i][j] != '"');
        
        return result;
    }
    else
        return argv[i];
}

static char *get_argument(int argc, char **argv, const char *name) {
    // Search every argument to find it
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], name) == 0 && argc >= i + 1)
            return parse_value(argv, i + 1);
    }

    // We have not found it
    return NULL;
}

bool format_image(FILE *image, int argc, char **argv) {
    // Get the name of the image and convert it to UTF-16
    char *name = get_argument(argc, argv, "--name");
    char16_t better_name[CFS_SB_NAME_LEN];
    ascii_to_utf16(name, better_name, CFS_SB_NAME_LEN);

    // Get the size of the image
    fseek(image, 0L, SEEK_END);
    uint64_t image_size = ftell(image);
    rewind(image);

    if (!format_image_cfs(image, image_size, better_name))
    {
        printf("Failed to format image!\n");
        return false;
    }

    return true;
}

uint64_t copy_file(FILE *image, int argc, char **argv)
{
    printf("Copy file not implemented\n");
    return 0;
}

uint64_t create_dir(FILE *image, int argc, char **argv)
{
    printf("Create directory not implemented\n");
    return 0;
}
