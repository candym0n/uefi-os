#ifndef STRING_H
#define STRING_H

#include <common/types.h>

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
char *strcpy(char *dest, const char *src);

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
int strcmp(const char *str1, const char *str2);

/**
 * @brief Finds the length in bytes of a string
 * 
 * @param str The string whose length is to be found
 * @return The length of the string
 */
size_t strlen(const char *str);

#endif // STRING_H