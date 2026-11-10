#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC */
extern s32 func_801783C8(s32 phase); /* @source 0x801783C8 */

/* @source 0x801F00D4
 * @behavior Decrements work byte 9 modulo 256. On zero, seeds words 0x14
 * and 0x20 with 0x40000 and 0x20000, offsets words 0x34 and 0x38 from
 * the record at 0x801EB4E0 using the phase samplers at angle (byte B & 31)
 * times 128 and a random scale from 28 through 43, and copies word 0x3C.
 * Seeds bytes 0x5D through 0x5F with random values from 5 through 12,
 * sets bytes 9 and A to 16, and increments state byte 2.
 * Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSeedRandomizedPositionTintAndAdvanceStateBmagicMagic12903_801F00D4(void) {
    u8 *work = g_battle_work;
    u8 count = work[9] - 1;
    s32 scale;
    s32 phase;

    work[9] = count;
    if (count == 0) {
        FIELD_REF(s32, g_battle_work, 0x14) = 0x40000;
        FIELD_REF(s32, g_battle_work, 0x20) = 0x20000;
        scale = (rand() & 15) + 28;
        phase = (g_battle_work[0xB] & 31) << 7;
        FIELD_REF(s32, g_battle_work, 0x34) = FIELD_REF(s32, D_801EB4E0, 0x34) + func_801782FC(phase) * scale;
        FIELD_REF(s32, g_battle_work, 0x38) = FIELD_REF(s32, D_801EB4E0, 0x38) + func_801783C8(phase) * scale;
        FIELD_REF(s32, g_battle_work, 0x3C) = FIELD_REF(s32, D_801EB4E0, 0x3C);
        g_battle_work[0x5D] = (rand() & 7) + 5;
        g_battle_work[0x5E] = (rand() & 7) + 5;
        g_battle_work[0x5F] = (rand() & 7) + 5;
        g_battle_work[9] = 16;
        g_battle_work[0xA] = 16;
        g_battle_work[2]++;
    }
}
