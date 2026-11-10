#include "bof3/bof3.h"

extern int rand(void);
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801EF2EC
 * @behavior Seed work byte 0xB with rand()'s low four bits, clear work bytes
 *           9 and 0xA through fresh scratchpad pointer-cell reads, then copy
 *           words 0x34, 0x38 and 0x3C from the record at D_801EB4E0 into the
 *           work object and increment its byte 2. Published in the overlay's
 *           handler table at 0x801F09A0; no arguments or return value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRandomWorkByteBClearBytes9AndAThenCopyRecordTripletAndAdvanceWorkByte2BmagicMagic09903_801EF2EC(void) {
    s32 noise;
    u8 *setB;
    u8 *clear9;
    u8 *clearA;
    s32 *source;
    s32 *dest;

    noise = rand();
    setB = g_battle_work;
    setB[0xB] = noise & 0xF;
    clear9 = g_battle_work;
    clear9[9] = 0;
    clearA = g_battle_work;
    clearA[0xA] = 0;
    source = (s32 *)D_801EB4E0;
    dest = (s32 *)g_battle_work;
    dest[13] = source[13];
    dest[14] = source[14];
    dest[15] = source[15];
    ((u8 *)dest)[2]++;
}
