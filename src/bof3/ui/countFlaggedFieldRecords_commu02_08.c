#include "bof3/bof3.h"

extern u8 D_8014496F[]; /* @source 0x8014496F @kind data */

/* @source 0x801D92B0
 * @behavior Counts records with bit 0 set in the seven bytes at 0x8014496F
 * spaced 0xA4 bytes apart. Returns the count as an unsigned byte.
 * @status partial
 * @match 64.71
 * @residual 11/17 instructions match; GCC replaces the ascending index with
 * a descending counter and bgez, swaps its register with the byte offset,
 * and emits 64 bytes instead of 68.
 */
u8 countFlaggedFieldRecords_commu02_08(void) {
    u8 count;
    s32 i;
    s32 offset;

    count = 0;
    offset = 0;
    for (i = 0; i < 7; i++) {
        if ((D_8014496F[offset] & 1) != 0) {
            count++;
        }
        offset += 0xA4;
    }
    return count;
}
