#include "options.h"

// i is the index in argv of the argument name
char *parse_value(char **argv, int i)
{
    int j = 0;
    void *result = malloc(72);

    if (argv[i][0] == '"')
    {
        do
        {
            ((char *)result)[j++] = argv[i][j];
        } while (argv[i][j] != '"');
        
        return result;
    }
    else
    {
        return argv[i];
    }
}

char *get_argument(int argc, char **argv, const char *name)
{
    // Search every argument to find it
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], name) == 0 && argc >= i + 1)
        {
            return parse_value(argv, i + 1);
        }
    }

    // We have not found it
    return NULL;
}

bool create_image(char *filename, int argc, char **argv)
{
    // Get argument values
    uint64_t img_size = string_to_sectors(get_argument(argc, argv, "--size"));

    // Check for valid inputs
    if (img_size <= 0)
    {
        printf("Image size of %lu too small!\n", img_size);
        return false;
    }

    // Open the file
    FILE *image = fopen(filename, "wb+");
    if (image == NULL)
    {
        printf("Failed to open file %s!", filename);
        return false;
    }

    // Write the Protective Master Boot Record (MBR)
    if (!write_mbr(image, img_size))
    {
        printf("Failed to write MBR!\n");
        return false;
    }
    // Write the GPT Header
    if (!write_gpt_headers(image, img_size))
    {
        printf("Failed to write GPT Header!\n");
        return false;
    }

    return true;
}

bool add_partition(FILE* image, int argc, char **argv)
{
    // Get arguments
    uint64_t size_lba = string_to_sectors(get_argument(argc, argv, "--size"));
    char *type = get_argument(argc, argv, "--type");
    char *name = get_argument(argc, argv, "--name");

    // Get the GUID given the type
    guid_t guid = get_guid(type);
    if (guid.clock_seq_hi_and_res == 0)
        return false;

    // Get the name in UCS-12
    char16_t *better_name = ascii_to_ucs2(name);

    // Add the partition
    if (!add_gpt_partition(image, size_lba, guid, better_name))
    {
        printf("Could not add partition %s!\n", name);
        fclose(image);
        free(better_name);
        return false;
    }

    // Cleanup
    fclose(image);
    free(better_name);
    return true;
}
