#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void (*D_801F07C4[])(void); /* @source 0x801F07C4 @kind data */
extern void func_801EFE20(void); /* @source 0x801EFE20 @kind function */

/* @source 0x801EFB10
 * @behavior Dispatches the handler indexed by work byte 2 through the table
 *           at 0x801F07C4. Re-reads the work pointer at 0x1F800044 after the
 *           indirect call and calls the local helper at 0x801EFE20 only when
 *           work bytes 0 and 2 are both nonzero. Takes no arguments and
 *           returns nothing. The helper's detailed role remains address-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenCallFE20WhenWorkBytes0And2SetBmagicMagic10303_801EFB10(void) {
  D_801F07C4[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_801EFE20();
  }
}
