#include "wc.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __WIN32
#include <io.h>
#include <stdio.h>
#include <windows.h>

int stdin_is_pipe_or_file_win(void)
{
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD type = GetFileType(h);

    if (type == FILE_TYPE_PIPE) return 1; // pipe (including redirected file)
    if (type == FILE_TYPE_DISK) return 1; // regular file
    return 0;                             // console or unknown
}
#endif

int stdin_is_pipe_or_file(void) {
    struct stat st;
    if (fstat(STDIN_FILENO, &st) == -1) {
        perror("fstat");
        return 0;               // treat as “no data”
    }

    /* S_ISFIFO – named pipe (FIFO) or pipe created by `|` */
    /* S_ISREG  – regular file (e.g., redirected from a file)   */
    return S_ISFIFO(st.st_mode) || S_ISREG(st.st_mode);
}

int print_usage(void);

int main(int argc, char** argv)
{
    if (argc < 2) {
        return print_usage();
    }

    char buf[1024];

    while (fgets(buf, sizeof buf, stdin)) {
        // TODO: handle stdin
    }

    const char* fname;
    Mode mode;

    if (argc == 2) {
        fname = argv[1];

        if (fname[0] == '-' && strlen(fname) == 2) {
            fprintf(stderr, "Invalid filename.\n");
            print_usage();
            return EXIT_FAILURE;
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
        size_t n_lines = 0, n_words = 0, n_bytes = 0;

        count_default(fp, &n_lines, &n_words, &n_bytes);

        printf("%6zu %6zu %6zu %s\n", n_lines, n_words, n_bytes, fname);
    } break;
    }

    fclose(fp);

    return EXIT_SUCCESS;
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
