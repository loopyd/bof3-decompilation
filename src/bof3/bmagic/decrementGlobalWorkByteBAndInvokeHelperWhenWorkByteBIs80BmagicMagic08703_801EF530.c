#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF530
 * @behavior Handler selected through the table at 0x801F050C by work byte 2.
 * Reads work byte B through the scratchpad pointer cell at 0x1F800044.
 * When that byte is 0x80, decrements byte B of the record published at
 * 0x801EB4E0 modulo 256, then calls the helper at 0x801E5988 without arguments.
 * The byte store precedes the call. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperWhenWorkByteBIs80BmagicMagic08703_801EF530(void) {
  if (g_battle_work[0xB] == 0x80) {
    D_801EB4E0[0xB]--;
    func_801E5988();
  }
}
