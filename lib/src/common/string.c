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

void ascii_to_utf16(char *ascii, char16_t *utf16, size_t utf16_len)
{
    // Convert each ASCII character to UTF-16 (stopping at a terminator or max char count)
    size_t i = 0;
    while (ascii[i] != '\0' && i < utf16_len - 1)
        utf16[i++] = (char16_t)ascii[i];

    // Fill in the rest with 0's
    while (++i < utf16_len)
        utf16[i] = 0;
}
