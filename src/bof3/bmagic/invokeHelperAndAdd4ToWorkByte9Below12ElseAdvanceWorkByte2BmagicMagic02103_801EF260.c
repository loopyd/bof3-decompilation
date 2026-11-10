#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801EF348(void); /* @source 0x801EF348 @kind function */

/* @source 0x801EF260
 * @behavior Tests unsigned scratchpad work-object byte 9 against 12. Below
 *           12, calls the overlay-local helper at 0x801EF348, reloads the
 *           work-object pointer cell, and adds four to byte 9 modulo 256.
 *           Otherwise increments byte 2 modulo 256 without calling the
 *           helper. Takes no arguments and returns nothing. The entry address
 *           appears at payload word 0x801EEC1C; the byte fields retain their
 *           offset-based names because their wider roles are not established.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperAndAdd4ToWorkByte9Below12ElseAdvanceWorkByte2BmagicMagic02103_801EF260(void) {
  if (g_battle_work[9] < 12) {
    func_801EF348();
    g_battle_work[9] += 4;
  } else {
    g_battle_work[2]++;
  }
}
