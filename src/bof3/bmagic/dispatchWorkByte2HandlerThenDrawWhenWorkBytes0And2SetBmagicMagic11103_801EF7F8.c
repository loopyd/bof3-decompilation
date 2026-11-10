#include "bof3/bof3.h"

/* @source 0x1F800044 @kind data */
extern u8 *g_battle_work;

/* Three step pointers at 0x801EFAB8, inside the textbin run at 0x801EFA9C:
 * 0x801EF870, 0x801EF8C4 and 0x801EF914. */
extern void (*D_801EFAB8[])(void); /* @source 0x801EFAB8 @kind data */

/* The target-local draw routine sets draw mode, emits a semitransparent
 * full-screen quad tinted by work byte 9, and restores draw mode. */
extern void func_801EF970(void); /* @source 0x801EF970 @kind function */

/* @source 0x801EF7F8
 * @behavior Calls the handler selected by unsigned work byte 2 from the
 * table at 0x801EFAB8 without a bounds test. Reloads the work pointer from
 * scratchpad cell 0x1F800044 after the call and invokes the local draw routine
 * at 0x801EF970 only when work bytes 0 and 2 are both nonzero. Reached through
 * the pointer at 0x801EFAA4; takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawWhenWorkBytes0And2SetBmagicMagic11103_801EF7F8(
    void) {
  D_801EFAB8[g_battle_work[2]]();
  if (g_battle_work[0] != 0 && g_battle_work[2] != 0) {
    func_801EF970();
  }
}
