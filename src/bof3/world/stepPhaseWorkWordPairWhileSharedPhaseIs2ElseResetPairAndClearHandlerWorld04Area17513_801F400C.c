#include "bof3/bof3.h"

extern u8 D_80143BB0; /* @source 0x80143BB0 @kind unknown */
extern s32 D_80143E6C; /* @source 0x80143E6C @kind unknown */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
extern u8* D_80146884; /* @source 0x80146884 @kind unknown */
extern s8 D_801F5360[]; /* @source 0x801F5360 @kind data */

/* @source 0x801F400C
 * @behavior Step handler installed by the pointer word at 0x801F5304 in the
 * overlay handler run at 0x801F52F0. While the shared phase byte D_80143BB0
 * equals 2 it reads the signed phase-step entry at 0x801F5360 selected by the
 * low nibble of D_80143E6C, adds that entry scaled by 0x800 to word 0x34 of
 * the work record reached through the pointer slot at 0x1F800044, subtracts
 * the independently read entry from word 0x38, then consumes two units of
 * countdown halfword 0x7E in the record published through 0x80146884. When
 * the phase byte differs from 2 it seeds word 0x34 with 0x2C0000, word 0x38
 * with 0x5C0000 and clears work handler byte 0x04. Takes no arguments and
 * returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepPhaseWorkWordPairWhileSharedPhaseIs2ElseResetPairAndClearHandlerWorld04Area17513_801F400C(void) {
  u8* work;
  u8* shared;
  u8* base;
  s32 phase;

  if (D_80143BB0 == 2) {
    phase = D_80143E6C & 0x0f;
    work = D_1F800044;
    *(s32*)(work + 0x34) += D_801F5360[phase] << 11;
    shared = D_80146884;
    *(s32*)(work + 0x38) -= D_801F5360[phase] << 11;
    *(u16*)(shared + 0x7e) -= 2;
  } else {
    base = D_1F800044;
    *(s32*)(base + 0x34) = 0x2C0000;
    *(s32*)(base + 0x38) = 0x5C0000;
    base[4] = 0;
  }
}
