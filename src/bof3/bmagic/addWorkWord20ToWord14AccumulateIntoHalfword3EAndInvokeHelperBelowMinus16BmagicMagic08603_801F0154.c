#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801F0154
 * @behavior Load the scratchpad work-object pointer once, add word 0x20 into
 *           word 0x14, and accumulate the updated word into halfword 0x3E.
 *           Re-read word 0x14 and invoke the shared helper at 0x801E5988 when
 *           its signed value is below -16. Takes no arguments and returns
 *           nothing. The pointer at 0x801F038C references this handler; no
 *           jal in this payload names it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addWorkWord20ToWord14AccumulateIntoHalfword3EAndInvokeHelperBelowMinus16BmagicMagic08603_801F0154(void) {
  u8 *work;

  work = g_battle_work;
  *(s32 *)(work + 0x14) += *(s32 *)(work + 0x20);
  *(u16 *)(work + 0x3E) += *(s32 *)(work + 0x14);
  if (*(s32 *)(work + 0x14) < -16) {
    func_801E5988();
  }
}
