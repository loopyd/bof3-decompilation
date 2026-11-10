#include "bof3/bof3.h"

extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801EF430(void); /* @source 0x801EF430 @kind function */

/* @source 0x801EF074
 * @behavior Copies byte 8 and words at offsets 0x34, 0x38 and 0x3C from the
 * record published through 0x801EB4E0 into the scratchpad work object, calls
 * the local helper at 0x801EF430, clears work bytes 9 and 0xA, and increments
 * work byte 2. The record and work cells are reloaded after the byte copy;
 * each post-call byte update also reloads the work cell. The entry is
 * published at payload offset 0x10; no in-image direct caller supplies args.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyRecordByte8AndWordTripletInvoke801EF430ClearBytes9AndAThenAdvanceWorkByte2BmagicMagic04003_801EF074(void) {
  s32 *src;
  s32 *dst;

  g_battle_work[8] = D_801EB4E0[8];
  src = (s32 *)D_801EB4E0;
  dst = (s32 *)g_battle_work;
  dst[13] = src[13];
  dst[14] = src[14];
  dst[15] = src[15];
  func_801EF430();
  g_battle_work[9] = 0;
  g_battle_work[10] = 0;
  g_battle_work[2]++;
}
