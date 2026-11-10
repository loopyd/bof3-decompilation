#include "bof3/bof3.h"

/* Shared helper outside this overlay; the call passes no arguments and does
 * not consume a return value. The generated function map binds its address. */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EF3E0
 * @behavior Decrement the scratchpad work object's unsigned byte 9 by one,
 *           testing the truncated stored value rather than reloading it. When
 *           that value is zero, invoke the shared helper at 0x801E5988. The
 *           pointer cell at 0x1F800044 is loaded once. Takes no arguments and
 *           returns nothing. The pointer word at 0x801F0368 references this
 *           handler; no jal in this payload names it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9AndInvokeHelperWhenZeroBmagicMagic08603_801EF3E0(void) {
  if (--g_battle_work[0x09] == 0) {
    func_801E5988();
  }
}
