#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t read_buffered(FILE* fp);
size_t read_byte_for_byte(FILE* fp);
int print_usage(void);

int main(int argc, char** argv)
{
    if (argc < 3) {
        return print_usage();
    }

    if (strncmp(argv[1], "-c", strlen("-c")) != 0) {
        return print_usage();
    }

    const char* fname = argv[2];

    FILE* fp = fopen(fname, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open '%s': %s (errno=%d)\n", fname, strerror(errno), errno);
        return EXIT_FAILURE;
    }

    printf("\t%zu %s\n", read_buffered(fp), fname);

    rewind(fp);

    printf("\t%zu %s\n", read_byte_for_byte(fp), fname);

    fclose(fp);

    return EXIT_SUCCESS;
}

size_t read_buffered(FILE* fp)
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
