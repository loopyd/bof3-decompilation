#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801F03D8(void); /* @source 0x801F03D8 @kind function */

/* @source 0x801F0214
 * @behavior Calls the target-local helper at 0x801F03D8, decrements unsigned
 * work byte 9, then reloads the scratchpad work pointer and increments work
 * byte 2 when the decremented byte is zero. The payload callback-table word
 * at offset 0x60 holds this entry. No argument or return value is used.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperThenDecrementWorkByte9AndAdvanceWorkByte2WhenZeroBmagicMagic06403_801F0214(void) {
  u8 *work;
  u8 countdown;
  u8 *object;

  func_801F03D8();
  work = g_battle_work;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    object = g_battle_work;
    object[2] += 1;
  }
}
