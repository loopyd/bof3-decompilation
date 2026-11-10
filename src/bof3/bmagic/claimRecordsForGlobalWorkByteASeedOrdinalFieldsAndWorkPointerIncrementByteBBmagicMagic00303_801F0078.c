#include "base/types.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern u8 magic00303Records[]; /* @source 0x801F0B38 @kind data */
extern u8 reserveFirstFreeRecordBmagicMagic00303_801F09C4(void); /* @source 0x801F09C4 @kind function */

/* @source 0x801F0078
 * @behavior Claims one 0x78-byte pool record per ordinal below the shared
 *           work record's byte 0xA. Clears record bytes 1 and 2, writes the
 *           ordinal to byte 4, (ordinal >> 1) & 7 to byte 0xB and
 *           ordinal * 4 + 1 to byte 9. Loads separate work and counter
 *           pointers from the shared cell, publishes the work pointer at
 *           record word 0x74 and increments counter byte 0xB. Reloads the
 *           work pointer and byte 0xA at every loop test.
 *           Does not check the allocator's 0xFF failure result. Takes no
 *           arguments and returns nothing. Field roles remain structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimRecordsForGlobalWorkByteASeedOrdinalFieldsAndWorkPointerIncrementByteBBmagicMagic00303_801F0078(void) {
    u8 ordinal;
    u8 index;
    u32 offset;
    u8 *work;
    u8 *counter;

    for (ordinal = 0; ordinal < D_801EB4E0[0xA]; ordinal++) {
        index = reserveFirstFreeRecordBmagicMagic00303_801F09C4();
        offset = index * 0x78;
        work = D_801EB4E0;
        magic00303Records[offset + 1] = 0;
        magic00303Records[offset + 2] = 0;
        magic00303Records[offset + 4] = ordinal;
        magic00303Records[offset + 0xB] = (ordinal >> 1) & 7;
        magic00303Records[offset + 9] = ordinal * 4 + 1;
        counter = D_801EB4E0;
        *(u8 **)&magic00303Records[offset + 0x74] = work;
        counter[0xB]++;
    }
}
