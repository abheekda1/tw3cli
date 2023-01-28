#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>

#include "w3sc.h"

typedef struct {
    char *dirname;
    char *name;
    void *data;
    uint32_t size;
} file_save_data_t;

void *save_file(void *arg) {
    file_save_data_t *file_save_data = (file_save_data_t *)arg;

    char* final_file_path = malloc(strlen(file_save_data->dirname) + 1 + strlen(file_save_data->name) + 1);
    strcpy(final_file_path, file_save_data->dirname);
    strcat(final_file_path, "/");
    strcat(final_file_path, file_save_data->name);

    // FILE *outfile = fopen(file_save_data->name, "w");
    FILE *outfile = fopen(final_file_path, "w");
    fwrite(file_save_data->data, file_save_data->size, 1, outfile);
    fclose(outfile);
    // printf("file_save_data{\n\tname: %s\n\tsize: %d\n}\n", file_save_data->name, file_save_data->size);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "not enough arguments! expected %d, got %d.\n", 1, argc - 1);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    int fd = fileno(file);

    fseek(file, 0, SEEK_END);
    size_t fsize = (size_t)ftell(file);
    rewind(file);

    void *mm_w3sc = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);

    w3sc_header_t *w3sc_header = (w3sc_header_t *)mm_w3sc;

    w3sc_t *w3sc = (w3sc_t *)malloc(sizeof(w3sc_t) + sizeof(w3sc_file_info_t *) * w3sc_header->file_count);
    w3sc->header = w3sc_header;

    printf("VERSION: %d\n", w3sc->header->version);
    printf("MAGIC:   %s\n", w3sc->header->magic);

    pthread_t *threads = malloc(w3sc->header->file_count * sizeof(pthread_t));

    char *dirname = malloc(strlen(argv[1]));
    int dirname_idx;
    for (dirname_idx = 0; dirname_idx < strlen(argv[1]) && argv[1][dirname_idx] != '.'; ++dirname_idx) {
        dirname[dirname_idx] = argv[1][dirname_idx];
    }
    dirname[dirname_idx] = 0;

    printf("Directory name: %s\n", dirname);
    mkdir(dirname, 0777);

    for (int i = 0; i < w3sc->header->file_count; i++) {
        w3sc->file_infos[i] = (w3sc_file_info_t *)(mm_w3sc + w3sc->header->file_infos_offset + sizeof(w3sc_file_info_t) * i);

        char *filename = (char *)(mm_w3sc + w3sc->header->file_names_offset + w3sc->file_infos[i]->name_offset);
        void *filedata = (void *)(mm_w3sc + w3sc->file_infos[i]->data_offset);
        uint32_t filesize = w3sc->file_infos[i]->data_size;

        file_save_data_t *file_save_data = malloc(sizeof(file_save_data_t));
        file_save_data->dirname = dirname;
        file_save_data->name = filename;
        file_save_data->data = filedata;
        file_save_data->size = filesize;

        pthread_create(&threads[i], NULL, save_file, file_save_data);


        fprintf(stdout, "extracting file %s!\n", filename);
    }

    for (int i = 0; i < w3sc->header->file_count; i++) {
        pthread_join(threads[i], NULL);
    }
}