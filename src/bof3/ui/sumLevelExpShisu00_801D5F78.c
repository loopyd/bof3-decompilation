#include "bof3/context.h"
#include "data/equipment.h"

/* The shared level table at 0x801CB8DC: 99 eight-byte LevelObject entries per
 * character row, each starting with the u16 experience threshold of that level.
 * The table sits below this overlay's load address, in the main executable, so
 * this overlay only reads it. The original computes a flat `row * 99 + index`
 * offset per iteration instead of walking a row pointer. */
extern LevelObject D_801CB8DC[][99]; /* @source 0x801CB8DC @kind data */

/* Body taken from the span at 0x801D1C44 in emi/etc/sisyou/00: a payload cmp of
 * the two 0x7C-byte spans showed only the `j` target word differing, by exactly
 * the 0x4334 offset delta between them, so the two are one body. */
/* @source 0x801D5F78
 * @behavior Sums the experience thresholds of one character row of the shared
 *           level table: the row and count arguments are masked to bytes, a
 *           count of 100 or greater returns -1 instead of summing, and the sum
 *           otherwise adds the leading u16 of the first count entries of row
 *           `row` from 0x801CB8DC. The 0x8-byte frame saves no register; the
 *           epilogue restores $sp and returns the sum in $v0.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 sumLevelExpShisu00_801D5F78(u8 row, u8 count)
{
    u8 index;
    s32 total;

    if (count >= 100) {
        return -1;
    }

    total = 0;
    for (index = 0; index < count; index++) {
        total += D_801CB8DC[row][index].exp;
    }
    return total;
}
