#include "bof3/bof3.h"

void func_800C238C(s32 arg0, s32 arg1);

extern u16 D_801463BE; /* @source 0x801463BE @kind unknown */

/* @source 0x800C2340
 * @behavior Three-byte overlay entry point: queues one slot-store record
 * through func_800C238C with its first two arguments masked to bytes (that
 * callee allocates a D_801EC330 record through func_801E590C(3, 5), copies the
 * 0x74-byte D_801EB630 work head into it and stores the two byte parameters),
 * then raises byte +0xB of the scratchpad work object at 0x1F800044 to 1 and
 * stores the third masked byte into the battle halfword 0x801463BE. The
 * sibling handler func_800C227C of this overlay raises the same +0xB byte and
 * writes 0x801463BE = 8 on its battle id 0x81 path, and
 * getAndClearBossBoss05516_800C2234 clears that +0xB byte while returning the
 * work pointer.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSlotThenRaisePendingFlagBossBoss05516_800C2340(s32 arg0, s32 arg1,
                                                         s32 arg2) {
  func_800C238C(arg0 & 0xFF, arg1 & 0xFF);
  SPAD_PTR_SLOT(u8, 0x44)[0xB] = 1;
  D_801463BE = arg2 & 0xFF;
}
