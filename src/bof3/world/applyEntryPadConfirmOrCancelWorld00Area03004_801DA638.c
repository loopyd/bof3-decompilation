#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 work-record pad decision step, the index-1 handler of the
 * record step table at 0x801E2248: when the shared pad state D_80145AA8 shares
 * a bit with the mask D_80145AC2 it clears the shared word D_801440C8, plays
 * sound cue 0x200 through func_8015DF18, disables the selected-record cue bytes
 * D_801E31FC and D_801E31F8 with 0xFF, requests resource 2 through
 * func_8014D8D4, then restarts the work record published at the scratchpad
 * cursor 0x1F800044 with the position word 0x410000 at offset 0x38, a cleared
 * slide count byte 9 and step byte 3 set to 2. Otherwise, when the pad state
 * shares a bit with the second mask D_80145AC4 and the shared mode byte
 * modeByte is clear, it clears the shared counter D_801440B1, stores 1 in the
 * record state byte 2 and returns the record to step 0. The shared helper
 * func_8014D978 runs last on every path.
 * @source 0x801DA638
 * @status partial
 * @match 91.07
 * @residual The cancel arm's byte-2 value 1 stays after the pointer load,
 * so the modeByte branch keeps a nop in its delay slot where the original
 * materialises that value first (one instruction, +4 bytes).
 */
void applyEntryPadConfirmOrCancelWorld00Area03004_801DA638(void) {
  u16 padMask;
  u8* work;
  u8* stateWork;

  padMask = D_80145AA8;
  if (padMask & D_80145AC2) {
    D_801440C8 = 0;
    func_8015DF18(0x200);
    D_801E31FC = 0xFF;
    D_801E31F8 = 0xFF;
    func_8014D8D4(2);
    work = D_1F800044;
    *(u32*)(work + 0x38) = 0x410000;
    work[9] = 0;
    D_1F800044[3] = 2;
  } else if ((padMask & D_80145AC4) && modeByte == 0) {
    stateWork = D_1F800044;
    D_801440B1 = 0;
    stateWork[2] = 1;
    D_1F800044[3] = 0;
  }
  func_8014D978();
}
