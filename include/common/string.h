#ifndef STRING_H
#define STRING_H

#include <stddef.h>

typedef unsigned short char16_t;
typedef unsigned int char32_t;

#define ISSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\v' || (c) == '\f' || (c) == '\r')
#define ISDIGIT(c) ((c) >= '0' && (c) <= '9')
#define ISALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define ISALNUM(c) (ISALPHA(c) || ISDIGIT(c))
#define ISLOWER(c) ((c) >= 'a' && (c) <= 'z')
#define ISUPPER(c) ((c) >= 'A' && (c) <= 'Z')
#define TOLOWER(c) (ISUPPER(c) ? (c) + 32 : (c))
#define TOUPPER(c) (ISLOWER(c) ? (c) - 32 : (c))

/**
 * @brief Copies the string pointed to by src to the buffer pointed to by dest.
 *
 * @param dest A pointer to the destination array where the content is to be copied.
 * @param src A pointer to the source string to be copied.
 * @return A pointer to the destination string dest.
 */
extern char *strcpy(char *dest, const char *src);

/**
 * @brief Compares two strings.
 *
 * @param str1 First string to be compared.
 * @param str2 Second string to be compared.
 * @return An integer less than, equal to, or greater than zero if str1 is found,
 *         respectively, to be less than, to match, or be greater than str2.
 *         More specifically:
 *         - < 0 means str1 is less than str2
 *         - = 0 means str1 is equal to str2
 *         - > 0 means str1 is greater than str2
 */
extern int strcmp(const char *str1, const char *str2);

/**
 * @brief Finds the length in bytes of a string
 * 
 * @param str The string whose length is to be found
 * @return The length of the string
 */
extern size_t strlen(const char *str);

/**
 * @brief Converts an ASCII string to UTF-16
 * 
 * @param ascii The original ascii string to convert
 * @param utf16 A pointer to an already allocated block of memory with a size of utf16_len * 2 bytes
 * @param utf16_len The length of the output utf16 string in words (not dog or therefore, I mean 2 bytes)
 * @return Nothing (void)
 */
extern void ascii_to_utf16(char *ascii, char16_t *utf16, size_t utf16_len);

#endif // STRING_H