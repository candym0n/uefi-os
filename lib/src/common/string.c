#include <common/string.h>
char *strcpy(char *dest, const char *src)
{
    char *dest_start = dest;

    // Continue copying until we hit the null terminator
    while (*src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
    }

    // Add the null terminator to the destination
    *dest = '\0';

    // Return pointer to the start of destination string
    return dest_start;
}

int strcmp(const char *str1, const char *str2)
{
    // Compare characters until a difference is found or one string ends
    while (*str1 != '\0' && *str1 == *str2)
    {
        str1++;
        str2++;
    }

    // Return the difference between the differing characters
    // or zero if both strings are equal
    return (int)(*str1) - (int)(*str2);
}

size_t strlen(const char *str)
{
    size_t len = 0;

    // Increment length until we hit the null terminator
    while (*(str++) != '\0')
        ++len;

    return len;
}
