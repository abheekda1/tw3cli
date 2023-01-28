#ifndef _W3SC_H
#define _W3SC_H

#include <stdint.h>

typedef char byte;

typedef struct {
    char magic[4];
    uint32_t version;
    byte dummy[8];
    uint32_t file_infos_offset;
    uint32_t file_count;
    uint32_t file_names_offset;
    uint32_t file_names_size;
} w3sc_header_t;

typedef struct {
    uint32_t name_offset;
    uint32_t data_offset;
    uint32_t data_size;
} w3sc_file_info_t;

typedef struct {
    w3sc_header_t *header;
    w3sc_file_info_t *file_infos[];
} w3sc_t;

#endif // _W3SC_H