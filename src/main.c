#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    MODE_BYTE,
    MODE_LINE,
    MODE_WORD,
    MODE_COUNT,
} Mode;

size_t count_by_byte(FILE* fp);
size_t read_byte_for_byte(FILE* fp);
size_t count_by_line(FILE* fp);
size_t count_by_word(FILE* fp);
int print_usage(void);

int main(int argc, char** argv)
{
    if (argc < 3) {
        return print_usage();
    }

    Mode mode;
    if (strncmp(argv[1], "-c", strlen("-c")) == 0) {
        mode = MODE_BYTE;
    } else if (strncmp(argv[1], "-l", strlen("-l")) == 0) {
        mode = MODE_LINE;
    } else if (strncmp(argv[1], "-w", strlen("-w")) == 0) {
        mode = MODE_WORD;
    } else {
        return print_usage();
    }

    const char* fname = argv[2];

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

    default:
        break;
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
    printf("usage: wc -c <file>\n");
    return EXIT_FAILURE;
}
