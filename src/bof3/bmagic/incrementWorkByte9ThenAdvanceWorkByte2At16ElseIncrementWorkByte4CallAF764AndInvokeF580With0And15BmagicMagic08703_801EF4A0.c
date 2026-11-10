#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_800AF764(void); /* @source 0x800AF764 @kind function */
extern void func_801EF580(s32 value); /* @source 0x801EF580 @kind function */

/* @source 0x801EF4A0
 * @behavior Increment work byte 9 modulo 256. When the result is 16,
 *           re-read the work pointer and increment work byte 2. Otherwise,
 *           re-read the work pointer, increment work byte 4, call 0x800AF764,
 *           and call 0x801EF580 with 0 followed by 15. The work pointer cell
 *           is at 0x1F800044. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementWorkByte9ThenAdvanceWorkByte2At16ElseIncrementWorkByte4CallAF764AndInvokeF580With0And15BmagicMagic08703_801EF4A0(void) {
  u8 *work;
  u8 counter;

  work = g_battle_work;
  counter = work[9] + 1;
  work[9] = counter;
  if (counter == 16) {
    g_battle_work[2]++;
  } else {
    g_battle_work[4]++;
    func_800AF764();
    func_801EF580(0);
    func_801EF580(15);
  }
}
