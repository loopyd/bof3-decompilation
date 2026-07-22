#ifndef EMI_H
#define EMI_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define EMI_MAX_ENTRIES 64

typedef enum {
    EMI_TYPE_RAM      = 0,
    EMI_TYPE_RAM_QUE1 = 1,
    EMI_TYPE_RAM_QUE2 = 2,
    EMI_TYPE_GFX      = 3,
    EMI_TYPE_SPECIAL4 = 4,
    EMI_TYPE_SPECIAL5 = 5,
    EMI_TYPE_VH       = 6,   /* VAB header */
    EMI_TYPE_VB       = 7,   /* VAB body */
    EMI_TYPE_AUX      = 8,   /* auxiliary audio (ADSR override) */
    EMI_TYPE_SEQ_SIDE = 9,   /* sequence-side payload */
    EMI_TYPE_SEQ      = 10,  /* SEP sequence */
} EmiType;

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

static inline int emi_check_magic(const uint8_t *data, size_t len)
{
    return len >= 16 && memcmp(data + 8, "MATH_TBL", 8) == 0;
}

static inline const char *emi_type_name(int type)
{
    switch (type) {
    case EMI_TYPE_RAM:      return "ram";
    case EMI_TYPE_RAM_QUE1: return "ram-q1";
    case EMI_TYPE_RAM_QUE2: return "ram-q2";
    case EMI_TYPE_GFX:      return "graphics";
    case EMI_TYPE_SPECIAL4: return "special4";
    case EMI_TYPE_SPECIAL5: return "special5";
    case EMI_TYPE_VH:       return "vab-header";
    case EMI_TYPE_VB:       return "vab-body";
    case EMI_TYPE_AUX:      return "aux-audio";
    case EMI_TYPE_SEQ_SIDE: return "seq-side";
    case EMI_TYPE_SEQ:      return "sequence";
    default:                return "unknown";
    }
}

#endif
