#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* @source 0x801F03EC
 * @behavior Clears work bytes 0xB and 9, calls the resident helper at
 *           0x8014D6B8 with 3, then increments work byte 2. The work pointer
 *           is reloaded from scratchpad cell 0x1F800044 for each operation.
 *           The entry is referenced by a pointer word at 0x801EEC18.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkBytesBAnd9Invoke8014D6B8With3ThenAdvanceWorkByte2BmagicMagic06703_801F03EC(void) {
  g_battle_work[0xB] = 0;
  g_battle_work[9] = 0;
  func_8014D6B8(3);
  g_battle_work[2]++;
}
