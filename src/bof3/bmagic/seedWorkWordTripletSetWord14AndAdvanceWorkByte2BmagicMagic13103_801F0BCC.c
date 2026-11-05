#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u32 word80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */
extern u32 *record801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* @source 0x801F0BCC
 * @behavior Copies the words at 0x80149308 and 0x8014930C to work offsets
 * 0x34 and 0x38, and record word 0x3C through the pointer at 0x801EB4E0
 * to work offset 0x3C. Sets work word 0x14 to 0x40, clears byte 0xB,
 * sets bytes 9 and 0xA to 0x10 and 1, then increments byte 2. Reloads
 * the scratchpad work pointer between the last three byte operations.
 * Takes no arguments and returns no value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkWordTripletSetWord14AndAdvanceWorkByte2BmagicMagic13103_801F0BCC(void) {
    u32 *work;
    u32 *source;
    u8 *set9;
    u8 *setA;
    u8 *step;

    work = (u32 *)g_battle_work;
    source = record801EB4E0;
    work[13] = word80149308[0];
    work[14] = D_8014930C[0];
    work[15] = source[15];
    work[5] = 0x40;
    ((u8 *)work)[0xB] = 0;
    set9 = g_battle_work;
    set9[9] = 0x10;
    setA = g_battle_work;
    setA[0xA] = 1;
    step = g_battle_work;
    step[2]++;
}
