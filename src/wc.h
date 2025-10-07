#ifndef WC_H_
#define WC_H_

#include <stddef.h>
#include <stdio.h>

#define MIN(a, b)                                                                                                      \
    ({                                                                                                                 \
        typeof(a) _a = (a);                                                                                            \
        typeof(b) _b = (b);                                                                                            \
        (_a < _b) ? _a : _b;                                                                                           \
    })

typedef enum {
    MODE_DEFAULT,
    MODE_BYTE,
    MODE_LINE,
    MODE_WORD,
    MODE_CHAR,
    MODE_COUNT,
} Mode;

typedef struct {
    FILE* fp;
    off_t file_start;
    off_t file_end;
    int thread_id;
    Mode mode;
} ThreadData;

void count_default(FILE* fp, size_t* n_lines, size_t* n_words, size_t* n_bytes);
size_t count_by_byte(FILE* fp);
size_t read_byte_for_byte(FILE* fp);
size_t count_by_line(FILE* fp);
size_t count_by_word(FILE* fp);
size_t count_by_char(FILE* fp);
void* thread_func(void* arg);

#endif // !WC_H_
