#include "bof3/bof3.h"

extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern s32 func_8017A138(s32 x, s32 y); /* @source 0x8017A138 */

/* @source 0x801EF12C
 * @behavior Computes helper 0x8017A138 of record-minus-work word differences
 *           at offsets 0x38 and 0x34, using the record pointer at 0x8014638C.
 *           Reloads the scratchpad work pointer and compares signed work word
 *           0xC divided by 64 with the helper result masked to 0xFFF and
 *           divided by 64. On equality writes 15 to work byte 0xA, reloads
 *           the pointer cell and increments work byte 2. Takes no arguments
 *           and returns nothing. Entry is word 4 of the handler table at
 *           0x801F0878. Record identity remains address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setWorkByteATo15AndAdvanceWorkByte2WhenHeadingBucketMatchesRecordToward8014638CBmagicMagic07803_801EF12C(void) {
  u8 *record;
  u8 *work;
  u8 *compareWork;
  s32 heading;

  record = D_8014638C;
  work = g_battle_work;
  heading = func_8017A138(*(s32 *)(record + 0x38) - *(s32 *)(work + 0x38),
                          *(s32 *)(record + 0x34) - *(s32 *)(work + 0x34));
  compareWork = g_battle_work;
  if (*(s32 *)(compareWork + 0xC) / 64 == (heading & 0xFFF) / 64) {
    compareWork[0xA] = 15;
    g_battle_work[2]++;
  }
}
