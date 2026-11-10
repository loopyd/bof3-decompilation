#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u32 D_80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */
extern void func_8015D71C(s32 selector); /* @source 0x8015D71C */

/* @source 0x801EFE70
 * @behavior Decrements work byte 9 modulo 256. On zero, calls the shared
 * helper at 0x8015D71C with 0x102, sets work words 0x34 and 0x38 to the
 * shared channel words at 0x80149308 and 0x8014930C plus work words 0xC
 * and 0x10, and clears work byte 0xB. Reloads the work pointer to set byte
 * 9 to 0x10, then reloads it again to clear words 0x14 and 0x20 and
 * advance byte 2. The table word at payload offset 0x2630 names this
 * handler. Takes no arguments and returns nothing; helper and field roles
 * not established by these accesses remain address- or offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenOffsetChannelPairClearByteBRearmByte9ClearWords14And20AndAdvanceWorkByte2BmagicMagic11403_801EFE70(void) {
  u32 *work;
  u32 *step;

  if (--g_battle_work[9] == 0) {
    func_8015D71C(0x102);
    work = (u32 *)g_battle_work;
    work[13] = D_80149308[0] + work[3];
    work[14] = D_8014930C[0] + work[4];
    ((u8 *)work)[0xB] = 0;
    g_battle_work[9] = 0x10;
    step = (u32 *)g_battle_work;
    step[5] = 0;
    step[8] = 0;
    ((u8 *)step)[2]++;
  }
}
