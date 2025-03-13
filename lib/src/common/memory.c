#include <common/memory.h>

void *memcpy(void *dest, const void *src, size_t n)
{
    // Cast void pointers to byte pointers for byte-by-byte copying
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    void *dest_start = dest;

    // Copy n bytes from src to dest
    while (n--)
    {
        *d++ = *s++;
    }

    // Return pointer to the start of destination memory
    return dest_start;
}

int memcmp(const void *ptr1, const void *ptr2, size_t n)
{
    // Cast void pointers to byte pointers for byte-by-byte comparison
    const unsigned char *p1 = (const unsigned char *)ptr1;
    const unsigned char *p2 = (const unsigned char *)ptr2;

    // Compare bytes until a difference is found or n bytes have been compared
    while (n--)
    {
        if (*p1 != *p2)
        {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }

    // All bytes are equal
    return 0;
}
