#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801F033C
 * @behavior Reads the scratchpad work pointer once and, when work byte 0xB
 *           is zero, decrements byte 0xB of the record selected by the shared
 *           pointer at 0x801EB4E0, then invokes 0x801E5988 with no arguments.
 *           Returns nothing. Published at payload offset 0x18EC.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementGlobalWorkByteBAndInvokeHelperWhenWorkByteBZeroBmagicMagic06703_801F033C(void) {
  u8 *work = g_battle_work;

  if (work[0xB] == 0) {
    D_801EB4E0[0xB]--;
    func_801E5988();
  }
}
