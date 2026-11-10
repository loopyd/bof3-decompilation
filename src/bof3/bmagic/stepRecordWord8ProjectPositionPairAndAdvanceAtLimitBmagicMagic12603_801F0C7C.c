#include "bof3/bof3.h"

extern s32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern u8 *D_801F22BC; /* @source 0x801F22BC @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern s16 D_1F800004; /* @source 0x1F800004 @kind data */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */

/* @source 0x801F0C7C
 * @behavior When bit 0 of the shared frame word at 0x80143E6C is set,
 * increments record byte 7. Adds 0x80000 to record word 8 and publishes
 * (byte 4 & 15) << 8 to scratchpad halfword 4. Calls 0x801782FC with
 * that angle, then 0x801783C8 with the signed scratchpad halfword. Each
 * returned sample times record byte 7 offsets the corresponding word
 * 0x34 or 0x38 of the record at 0x801EB4E0 into words 0x10 and 0x14.
 * Adds word 8 to word 0x18 and advances byte 2 when word 8 exceeds
 * 0x7FFFFF signed. Reloads the record pointer after each call. Callback
 * at table word 0x801F12B4; takes no arguments and returns nothing.
 * Other record roles remain offset-based.
 * @status partial
 * @match 94.59
 * @residual 70 of 74 instructions at the original 296 bytes. The first
 * post-call record pointer uses a2 instead of the original a0.
 */
void stepRecordWord8ProjectPositionPairAndAdvanceAtLimitBmagicMagic12603_801F0C7C(void) {
    u8 *record;
    s16 *angle;
    s32 sample;
    s32 direction;

    if (D_80143E6C & 1) {
        D_801F22BC[7]++;
    }
    record = D_801F22BC;
    angle = &D_1F800004;
    direction = record[4] & 15;
    *(s32 *)(record + 8) += 0x80000;
    *angle = (u8)direction << 8;
    sample = func_801782FC(direction << 8);
    record = D_801F22BC;
    *(s32 *)(record + 0x10) = *(s32 *)(D_801EB4E0 + 0x34) + sample * record[7];
    sample = func_801783C8(*angle);
    record = D_801F22BC;
    *(s32 *)(record + 0x14) = *(s32 *)(D_801EB4E0 + 0x38) + sample * record[7];
    *(s32 *)(record + 0x18) += *(s32 *)(record + 8);
    if (*(s32 *)(record + 8) > 0x7FFFFF) {
        record[2]++;
    }
}
