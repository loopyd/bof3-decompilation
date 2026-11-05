#include "bof3/bof3.h"

extern u8 D_8014496F[]; /* @source 0x8014496F @kind data */

/* @source 0x801D92B0
 * @behavior Counts records with bit 0 set in the seven bytes at 0x8014496F
 * spaced 0xA4 bytes apart. Returns the count as an unsigned byte.
 * @status partial
 * @match 76.47
 * @residual 13/17 instructions match; the ascending index and byte offset
 * use v1/a0 rather than the original a0/v1. Function size matches at 68 bytes.
 */
u8 countFlaggedFieldRecords_commu02_08(void) {
    u8 count;
    s32 i;
    s32 offset;

    count = 0;
    offset = 0;
    i = 0;
next_record:
    if ((D_8014496F[offset] & 1) != 0) {
        count++;
    }
    offset += 0xA4;
    i++;
    if (i < 7) {
        goto next_record;
    }
    return count;
}
