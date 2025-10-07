#include "wc.h"
#include <ctype.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Return the number of lines, words and bytes in a file the same as *nix's wc command.
 *
 * \param fp File pointer
 * \param n_lines An out variable for the number of lines
 * \param n_words An out variable for the number of words
 * \param n_bytes An out variable for the number of bytes
 */
int count_default(FILE* fp, size_t* tot_lines, size_t* tot_words, size_t* tot_chars)
{
    size_t chunk_size = 1024;
    unsigned char* buf = (unsigned char*)malloc(chunk_size);
    if (!buf) {
        perror("Failed to allocate memory");
        fclose(fp);
        return EXIT_FAILURE;
    }

    char c, prev_c;

    while ((c = fgetc(fp)) != EOF) {
        if (isspace(c) && !isspace(prev_c)) {
            (*tot_words)++;
        }
        prev_c = c;

        if (c == '\n') {
            (*tot_lines)++;
        }

        (*tot_chars)++;
    }

    return EXIT_SUCCESS;
}

/*
 * Return the number in bytes of a file with buffered reads.
 *
 * \param fp File pointer
 * \return The number of bytes
 */
size_t count_by_byte(FILE* fp)
{
    size_t chunk_size = 1024;
    unsigned char* buf = (unsigned char*)malloc(chunk_size);
    if (!buf) {
        perror("Failed to allocate memory");
        fclose(fp);
        return EXIT_FAILURE;
    }

    size_t n_read, tot_bytes = 0;
    while ((n_read = fread(buf, 1, chunk_size, fp)) > 0) {
        tot_bytes += n_read;
    }
    if (ferror(fp)) {
        perror("Failed reading data from file");
    }

    free(buf);

    return tot_bytes;
}

/*
 * Return the number in lines of a file.
 *
 * \param fp File pointer
 * \return The number of lines
 */
size_t count_by_line(FILE* fp)
{
    size_t chunk_size = 1024;
    char* buf = (char*)malloc(chunk_size);
    if (!buf) {
        perror("Failed to allocate memory");
        fclose(fp);
        return EXIT_FAILURE;
    }

    size_t n_read, tot_lines = 0;

    while (fgets(buf, chunk_size, fp)) {
        tot_lines++;
    }

    free(buf);

    return tot_lines;
}

/*
 * Return the number in words of a file.
 *
 * \param fp File pointer
 * \return The number of words
 */
size_t count_by_word(FILE* fp)
{
    size_t chunk_size = 1024;
    char* buf = (char*)malloc(chunk_size);
    if (!buf) {
        perror("Failed to allocate memory");
        fclose(fp);
        return EXIT_FAILURE;
    }

    size_t n_read, tot_words = 0;
    char c, prev_c;

    while ((c = fgetc(fp)) != EOF) {
        if (isspace(c) && !isspace(prev_c)) {
            tot_words++;
        }
        prev_c = c;
    }

    free(buf);

    return tot_words;
}

/*
 * Return the number in characters of a file.
 *
 * \param fp File pointer
 * \return The number of characters
 */
size_t count_by_char(FILE* fp)
{
    size_t chunk_size = 1024;
    char* buf = (char*)malloc(chunk_size);
    if (!buf) {
        perror("Failed to allocate memory");
        fclose(fp);
        return EXIT_FAILURE;
    }

    size_t n_read, tot_chars = 0;
    char c;

    while ((c = fgetc(fp)) != EOF) {
        tot_chars++;
    }

    free(buf);

    return tot_chars;
}

/*
 * Return the number in bytes of a file, counting byte by byte.
 *
 * \param fp File pointer
 * \return The number of bytes
 */
size_t read_byte_for_byte(FILE* fp)
{
    size_t tot_bytes = 0;
    int ch;

    while ((ch = fgetc(fp)) != EOF) {
        unsigned char byte = (unsigned char)ch;
        tot_bytes++;
    }

    return tot_bytes;
}
