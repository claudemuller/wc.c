#include "wc.h"
#include <ctype.h>
#include <pthread.h>
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
void count_default(FILE* fp, size_t* n_lines, size_t* n_words, size_t* n_bytes)
{
    const int N_THREADS = 3;
    pthread_t threads[N_THREADS];
    ThreadData thread_data[N_THREADS];
    Mode modes[N_THREADS] = {MODE_LINE, MODE_WORD, MODE_CHAR};

    int fd = fileno(fp);
    if (fd == -1) {
        perror("Failed to get file descriptor.");
        // TODO: handle errro
        return;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == (off_t)-1) {
        perror("lseek");
        close(fd);
        exit(EXIT_FAILURE);
    }
    lseek(fd, 0, SEEK_SET);

    size_t j = 0;

    /* Create threads */
    for (int i = 0; i < N_THREADS; ++i) {
        thread_data[i].thread_id = i;
        thread_data[i].mode = modes[j++];
        thread_data[i].fp = fp;
        thread_data[i].file_start = 0;
        thread_data[i].file_end = file_size;

        int rc = pthread_create(&threads[i], NULL, thread_func, (void*)&thread_data[i]);
        if (rc != 0) {
            fprintf(stderr, "Error: pthread_create failed for thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    /* Wait for all threads to finish */
    for (int i = 0; i < N_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    // *n_bytes = count_by_byte(fp);
    //
    // rewind(fp);
    // *n_lines = count_by_line(fp);
    //
    // rewind(fp);
    // *n_words = count_by_word(fp);
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

size_t count_by_char_pos_independent_read(FILE* fp, off_t file_start, off_t file_end)
{
    int fd = fileno(fp);
    if (fd == -1) {
        perror("Failed to get file descriptor.");
        return EXIT_FAILURE;
    }

    size_t chunk_size = 1024;
    char* buf = (char*)malloc(chunk_size);
    if (!buf) {
        perror("Failed to allocate memory");
        fclose(fp);
        return -1;
    }

    off_t pos = file_start;
    size_t tot_chars = 0;
    char c;

    while (pos < file_end) {
        // size_t to_read = MIN(sizeof(buf), file_end - pos);
        ssize_t n = pread(fd, buf, 1, pos);

        // Error or EOF
        if (n <= 0) {
            // TODO: handle this
            printf("EOF or error\n");
            break;
        }

        pos += n;

        if (isspace(buf[0])) {
            tot_chars++;
        }
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

void* thread_func(void* arg)
{
    ThreadData* data = (ThreadData*)arg;

    switch (data->mode) {
    case MODE_LINE: {
        size_t n = count_by_char_pos_independent_read(data->fp, data->file_start, data->file_end);
        if (n < 0) {
            // TODO: handle error
        }

        printf("Thread %d says: %zu\n", data->thread_id, n);
    } break;

    case MODE_WORD: {
        size_t n = count_by_char_pos_independent_read(data->fp, data->file_start, data->file_end);
        if (n < 0) {
            // TODO: handle error
        }

        printf("Thread %d says: %zu\n", data->thread_id, n);
    } break;

    case MODE_CHAR: {
        size_t n = count_by_char_pos_independent_read(data->fp, data->file_start, data->file_end);
        if (n < 0) {
            // TODO: handle error
        }

        printf("Thread %d says: %zu\n", data->thread_id, n);
    } break;
    }

    for (volatile long i = 0; i < 100000000L; ++i) {
    }

    return NULL;
}
