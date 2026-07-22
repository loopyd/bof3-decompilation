#ifndef EMI_H
#define EMI_H

#include <stdint.h>
#include <stddef.h>

#define EMI_MAX_ENTRIES 64
#define EMI_TYPE_VH  6
#define EMI_TYPE_VB  7
#define EMI_TYPE_AUX 8
#define EMI_TYPE_SEQ 10

typedef struct {
    uint32_t size;
    uint32_t offset;
    uint16_t type;
} EmiEntry;

typedef struct {
    int count;
    EmiEntry entries[EMI_MAX_ENTRIES];
    const uint8_t *data;
    size_t data_len;
} EmiFile;

int emi_parse(const uint8_t *data, size_t len, EmiFile *emi);
const uint8_t *emi_find_type(const EmiFile *emi, int type, uint32_t *size);

#endif
