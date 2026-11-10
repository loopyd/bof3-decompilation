#include "bof3/bof3.h"

extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern u8 *D_801F22BC; /* @source 0x801F22BC @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */
extern void clearRecordBytes0Through3BmagicMagic12603_801F116C(void); /* @source 0x801F116C */

/* @source 0x801F0DA4
 * @behavior Increments record byte 7 when shared frame word bit 0 is set.
 * Publishes (record byte 4 & 15) << 8 to scratchpad halfword 4 and calls
 * 0x801782FC with that angle, then 0x801783C8 with the signed halfword.
 * Each returned sample times record byte 7 offsets shared record words
 * 0x34 and 0x38 into record words 0x10 and 0x14. Adds word 8 to word
 * 0x18 and decrements byte 6. At zero, decrements shared record byte B
 * and clears record bytes 0 through 3. Reloads the record cell after
 * each sample call. Table callback at 0x801F12B8; no arguments or result.
 * Other record roles remain offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void projectRecordPositionPairAccumulateWord18CountDownByte6AndClearAtZeroBmagicMagic12603_801F0DA4(void) {
    u8 *record;
    u8 *record2;
    u8 *record3;
    s16 *angle;
    s32 sample;
    s32 direction;

    if (D_80143E6C & 1) {
        D_801F22BC[7]++;
    }
    record = D_801F22BC;
    angle = &D_1F800004;
    direction = record[4] & 15;
    *angle = (u8)direction << 8;
    sample = func_801782FC(direction << 8);
    record2 = D_801F22BC;
    *(s32 *)(record2 + 0x10) = *(s32 *)(D_801EB4E0 + 0x34) + sample * record2[7];
    sample = func_801783C8(*angle);
    record3 = D_801F22BC;
    *(s32 *)(record3 + 0x14) = *(s32 *)(D_801EB4E0 + 0x38) + sample * record3[7];
    *(s32 *)(record3 + 0x18) += *(s32 *)(record3 + 8);
    if (--record3[6] == 0) {
        D_801EB4E0[0xB]--;
        clearRecordBytes0Through3BmagicMagic12603_801F116C();
    }
}
