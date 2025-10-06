#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    MODE_DEFAULT,
    MODE_BYTE,
    MODE_LINE,
    MODE_WORD,
    MODE_CHAR,
    MODE_COUNT,
} Mode;

size_t count_by_byte(FILE* fp);
size_t read_byte_for_byte(FILE* fp);
size_t count_by_line(FILE* fp);
size_t count_by_word(FILE* fp);
size_t count_by_char(FILE* fp);
int print_usage(void);

int main(int argc, char** argv)
{
    if (argc < 2) {
        return print_usage();
    }

    const char* fname;
    Mode mode;

    if (argc == 2) {
        fname = argv[1];

        if (fname[0] == '-' && strlen(fname) == 2) {
            fprintf(stderr, "Invalid filename.\n");
            return print_usage();
        }

        mode = MODE_DEFAULT;
    } else {
        fname = argv[2];

        if (strncmp(argv[1], "-c", strlen("-c")) == 0) {
            mode = MODE_BYTE;
        } else if (strncmp(argv[1], "-l", strlen("-l")) == 0) {
            mode = MODE_LINE;
        } else if (strncmp(argv[1], "-w", strlen("-w")) == 0) {
            mode = MODE_WORD;
        } else if (strncmp(argv[1], "-m", strlen("-m")) == 0) {
            mode = MODE_CHAR;
        } else {
            return print_usage();
        }
    }

    FILE* fp = fopen(fname, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open '%s': %s (errno=%d)\n", fname, strerror(errno), errno);
        return EXIT_FAILURE;
    }

    switch (mode) {
    case MODE_BYTE: {
        printf("\t%zu %s\n", count_by_byte(fp), fname);
    } break;

    case MODE_LINE: {
        printf("\t%zu %s\n", count_by_line(fp), fname);
    } break;

    case MODE_WORD: {
        printf("\t%zu %s\n", count_by_word(fp), fname);
    } break;

    case MODE_CHAR: {
        printf("\t%zu %s\n", count_by_char(fp), fname);
    } break;

    default: {
        printf("\t%zu %s\n", count_by_byte(fp), fname);
        rewind(fp);
        printf("\t%zu %s\n", count_by_line(fp), fname);
        rewind(fp);
        printf("\t%zu %s\n", count_by_word(fp), fname);
    } break;
    }

    fclose(fp);

    return EXIT_SUCCESS;
}

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

// TODO: confirm output with wc
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

int print_usage(void)
{
    printf("usage: wc [mode] <file>\n");
    printf("\tModes:\n");
    printf("\t\t-c number of bytes\n");
    printf("\t\t-l number of lines\n");
    printf("\t\t-w number of words\n");
    printf("\t\t-m number of characters\n");

    return EXIT_SUCCESS;
}
