#include <string.h>
#include "emi.h"
#include "util.h"

int emi_parse(const uint8_t *data, size_t len, EmiFile *emi)
{
    uint32_t count, i;
    size_t off;

    memset(emi, 0, sizeof(*emi));
    if (len < 16)
        return -1;

    count = rd_u32le(data);
    if (memcmp(data + 8, "MATH_TBL", 8) != 0)
        return -1;
    if (count > EMI_MAX_ENTRIES)
        count = EMI_MAX_ENTRIES;

    emi->data = data;
    emi->data_len = len;
    emi->count = (int)count;

    off = 0x800;
    for (i = 0; i < count; i++) {
        const uint8_t *toc = data + 16 + i * 16;
        uint32_t size;

        if (16 + (i + 1) * 16 > len)
            break;

        size = rd_u32le(toc);
        emi->entries[i].size   = size;
        emi->entries[i].offset = (uint32_t)off;
        emi->entries[i].type   = rd_u16le(toc + 12);

        off += ((size_t)size + 0x7FF) & ~(size_t)0x7FF;
    }

    return 0;
}

const uint8_t *emi_find_type(const EmiFile *emi, int type, uint32_t *size)
{
    int i;
    for (i = 0; i < emi->count; i++) {
        if (emi->entries[i].type == type) {
            if (size)
                *size = emi->entries[i].size;
            return emi->data + emi->entries[i].offset;
        }
    }
    return NULL;
}
