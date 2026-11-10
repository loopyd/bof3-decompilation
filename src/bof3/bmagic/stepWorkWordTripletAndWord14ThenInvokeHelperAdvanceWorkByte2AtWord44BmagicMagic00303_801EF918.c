#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC */

/* @source 0x801EF918
 * @behavior Adds work words 0xC, 0x10 and 0x14 to words 0x34, 0x38 and
 *           0x3C respectively, then adds word 0x20 to word 0x14. Calls the
 *           shared helper at 0x8014DAEC and discards its result. Reloads the
 *           scratchpad work pointer and increments byte 2 when word 0x3C
 *           equals word 0x44. Takes no arguments and returns nothing. The
 *           entry is referenced at payload offset 0x1C; field roles remain
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkWordTripletAndWord14ThenInvokeHelperAdvanceWorkByte2AtWord44BmagicMagic00303_801EF918(void) {
  u8 *work = g_battle_work;
  u8 *updatedWork;

  *(u32 *)(work + 0x34) += *(u32 *)(work + 0xC);
  *(u32 *)(work + 0x38) += *(u32 *)(work + 0x10);
  *(u32 *)(work + 0x3C) += *(u32 *)(work + 0x14);
  *(u32 *)(work + 0x14) += *(u32 *)(work + 0x20);
  func_8014DAEC();
  updatedWork = g_battle_work;
  if (*(u32 *)(updatedWork + 0x3C) == *(u32 *)(updatedWork + 0x44)) {
    updatedWork[2]++;
  }
}
