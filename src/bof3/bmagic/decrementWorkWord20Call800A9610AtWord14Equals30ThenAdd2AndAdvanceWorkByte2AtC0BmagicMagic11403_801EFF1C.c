#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 D_80146384; /* @source 0x80146384 @kind data */
extern void func_800A9610(u8 selector, s32 flags); /* @source 0x800A9610 @kind function */

/* @source 0x801EFF1C
 * @behavior Subtracts 5 from work word 0x20. When work word 0x14 is 0x30,
 * calls 0x800A9610 with byte 0x80146384 and 0x10. Reloads the work pointer,
 * adds 2 to word 0x14, and when the new value is 0xC0 sets byte 0xB to 1
 * and advances byte 2 through a fresh work-pointer read. The table word at
 * payload offset 0x2634 names this no-argument handler. Field roles remain
 * offset-based; the handler returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkWord20Call800A9610AtWord14Equals30ThenAdd2AndAdvanceWorkByte2AtC0BmagicMagic11403_801EFF1C(void) {
  u32 *work;
  u32 *step;

  work = (u32 *)g_battle_work;
  work[8] -= 5;
  if (work[5] == 0x30) {
    func_800A9610(D_80146384, 0x10);
  }
  step = (u32 *)g_battle_work;
  if ((step[5] += 2) == 0xC0) {
    ((u8 *)step)[0xB] = 1;
    g_battle_work[2]++;
  }
}
