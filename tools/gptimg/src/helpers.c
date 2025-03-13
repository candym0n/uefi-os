#include "helpers.h"

void pad_lba(FILE *image)
{
    // Define a block full of zeros
    uint8_t zero[INTERNAL_UNIT] = {0};

    // Write this sector enough times to fill a block
    for (uint8_t i = 0; i < (lba_size - INTERNAL_UNIT) / INTERNAL_UNIT; ++i)
    {
        fwrite(zero, INTERNAL_UNIT, 1, image);
    }
}

uint64_t string_to_sectors(const char *size)
{
    if (size == NULL || strlen(size) == 0)
    {
        return 0; // Handle empty or null input
    }

    uint64_t num = 0;
    char unit = 0;
    int i = 0;

    // Skip leading whitespace
    while (ISSPACE(size[i]))
    {
        i++;
    }

    // Extract the numerical part
    while (ISDIGIT(size[i]))
    {
        num = num * 10 + (size[i] - '0');
        i++;
    }

    // Skip any whitespace between number and unit
    while (ISSPACE(size[i]))
    {
        i++;
    }

    // Extract the unit (K, M, G, T, etc. - case-insensitive)
    unit = TOUPPER(size[i]);

    uint64_t multiplier = 1;

    switch (unit)
    {
    case 'K':
        multiplier = 1024;
        break;
    case 'M':
        multiplier = 1024 * 1024;
        break;
    case 'G':
        multiplier = 1024ULL * 1024 * 1024;
        break;
    case 'T':
        multiplier = 1024ULL * 1024 * 1024 * 1024; // Use ULL to prevent overflow
        break;
    case '\0': // No unit specified, assume bytes
        multiplier = 1;
        break;
    default:
        return 0; // Invalid unit
    }

    return num * multiplier / lba_size;
}

guid_t random_guid(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec * 1000000 + tv.tv_usec);
    uint8_t rand_arr[16] = {0};

    for (uint8_t i = 0; i < sizeof rand_arr; i++)
        rand_arr[i] = rand() & 0xFF; // Equivalent to modulo 256

    // Fill out GUID
    guid_t result = {
        .time_lo = *(uint32_t *)&rand_arr[0],
        .time_mid = *(uint16_t *)&rand_arr[4],
        .time_hi_and_ver = *(uint16_t *)&rand_arr[6],
        .clock_seq_hi_and_res = rand_arr[8],
        .clock_seq_lo = rand_arr[9],
        .node = {rand_arr[10], rand_arr[11], rand_arr[12], rand_arr[13],
                 rand_arr[14], rand_arr[15]},
    };

    // Fill out version bits - version 4
    result.time_hi_and_ver &= ~(1 << 15); // 0b_0_111 1111
    result.time_hi_and_ver |= (1 << 14);  // 0b0_1_00 0000
    result.time_hi_and_ver &= ~(1 << 13); // 0b11_0_1 1111
    result.time_hi_and_ver &= ~(1 << 12); // 0b111_0_ 1111

    // Fill out variant bits
    result.clock_seq_hi_and_res |= (1 << 7);  // 0b_1_000 0000
    result.clock_seq_hi_and_res |= (1 << 6);  // 0b0_1_00 0000
    result.clock_seq_hi_and_res &= ~(1 << 5); // 0b11_0_1 1111

    return result;
}

uint8_t *guid_to_bytes(guid_t guid)
{
    // Allocate memory for the result
    uint8_t *result = (uint8_t *)malloc(sizeof(guid_t));

    if (result == NULL)
    {
        printf("Failed to allocate memory for GUID conversion.\n");
        return NULL;
    }

    // Copy the guid_t structure to the result
    memcpy(result, &guid, sizeof(guid_t));

    return result;
}

char16_t *ascii_to_ucs2(const char *ascii)
{
    if (!ascii)
        return NULL;

    // Get length of input string
    size_t len = strlen(ascii);

    // Allocate buffer for UCS-2 string (including null terminator)
    char16_t *ucs2 = malloc((len + 1) * sizeof(char16_t));
    if (!ucs2)
        return NULL;

    // Convert each ASCII character to UCS-2
    // Since ASCII is limited to 7 bits, this is a direct mapping
    for (size_t i = 0; i < len; i++)
    {
        ucs2[i] = (char16_t)(unsigned char)ascii[i];
    }

    // Null terminate the string
    ucs2[len] = 0;

    return ucs2;
}
