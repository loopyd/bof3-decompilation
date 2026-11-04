#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s16 D_801F004C[]; /* @source 0x801F004C @kind data */
extern s16 D_801F004E[]; /* @source 0x801F004E @kind data */

/* @source 0x801EF06C
 * @behavior Copies words 0x34, 0x38 and 0x3C from the record published at
 *           0x801EB4E0 to the scratchpad work record. Seeds work words 0x0C
 *           and 0x10 from source word 0x0C plus signed halfwords indexed by
 *           work byte 4 at 0x801F004C and 0x801F004E, respectively. Clears
 *           work bytes 0x0B, 9 and 0x0A, reloading the scratchpad pointer
 *           between store groups, then increments byte 2. Takes no arguments
 *           and returns nothing.
 * @status partial
 * @match 91.30
 * @residual Unused 8-byte SP adjustment around the final lbu/addiu/sb.
 *           No stack slot is read or written anywhere in the original 184
 *           bytes. The first 40 instructions are byte-identical; there is
 *           no access, call or ABI difference.
 */
void copyWorkTripletSeedOffsetPairAndAdvanceWorkByte2BmagicMagic10503_801EF06C(void) {
    u32 *source;
    u32 *work;
    u8 *clear9;
    u8 *clearA;
    u8 *step;

    source = (u32 *)D_801EB4E0;
    work = (u32 *)g_battle_work;
    work[13] = source[13];
    work[14] = source[14];
    work[15] = source[15];
    work[3] = source[3] + D_801F004C[((u8 *)work)[4]];
    work[4] = source[3] + D_801F004E[((u8 *)work)[4]];
    ((u8 *)work)[0xB] = 0;
    clear9 = g_battle_work;
    clear9[9] = 0;
    clearA = g_battle_work;
    clearA[0xA] = 0;
    step = g_battle_work;
    step[2]++;
}
