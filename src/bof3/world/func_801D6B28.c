#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 selection-panel marker refresh: runs the shared panel
 * reset func_8014D4E0, submits the tpage draw mode (0, 1), draws the three
 * panel lines through func_801E0DCC (0x15/0x16/0x11 at 1 with widths 0x32/0xBC/
 * 0x36 and length 0x96), clamps the work-record x word 0x0C to 0x39..0x79 and
 * its companion word 0x10 to 0x41..0x94 in place, then redraws the selection
 * marker row: func_801D3244 receives the clamped x word (as a signed halfword),
 * 0xB3, 0x20, the work-record byte 6 low three bits doubled and a final mode
 * flag that is 1 when word 0x10 is below word 0x0C or word 0x0C plus 0x20 is
 * below word 0x10 and 0 otherwise; func_801D2C34 then receives the clamped
 * companion word as a signed halfword, 0xB3, arg0's byte as a flag and 1.
 * @source 0x801D6B28
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The published cursor cell 0x1F800044 is volatile, so the original re-reads
 * it once per access group: the x clamp, the companion clamp, the marker-mode
 * comparison and the trailing halfword read. Keeping each group's pointer in
 * its own block-scoped local reproduces the original's $a1/$a2/$v1/$v0 read
 * registers and the schedule that issues the next group's read before the
 * previous group's store. Both clamps are conditional expressions so the
 * original's separate merge register ($a0) with the pass-through copy in the
 * delay slot survives; folding them back into plain if/else assignments merges
 * the load and result registers and loses that copy. The marker mode is passed
 * as two branch calls, not a ternary argument, because the original duplicates
 * the whole argument setup in both arms.
 */
void func_801D6B28(u8 arg0) {
  s32 left;
  s32 right;

  func_8014D4E0();
  submitTpageDrawMode(0, 1);
  func_801E0DCC(0x15, 1, 0x32, 0x96);
  func_801E0DCC(0x16, 1, 0xBC, 0x96);
  func_801E0DCC(0x11, 1, 0x36, 0x9C);

  {
    u8* work;
    u8* next;
    u8* check;

    work = D_1F800044;
    left = *(s32*)(work + 0x0C);
    left = (left < 0x39) ? 0x39 : ((left >= 0x7A) ? 0x79 : left);
    next = D_1F800044;
    *(s32*)(work + 0x0C) = left;

    right = *(s32*)(next + 0x10);
    right = (right < 0x41) ? 0x41 : ((right >= 0x95) ? 0x94 : right);
    check = D_1F800044;
    *(s32*)(next + 0x10) = right;

    if (*(s32*)(check + 0x10) < *(s32*)(check + 0x0C) ||
        *(s32*)(check + 0x0C) + 0x20 < *(s32*)(check + 0x10)) {
      func_801D3244((s16)*(s32*)(check + 0x0C), 0xB3, 0x20,
                    (s8)((check[6] & 7u) << 1), 1, 1);
    } else {
      func_801D3244((s16)*(s32*)(check + 0x0C), 0xB3, 0x20,
                    (s8)((check[6] & 7u) << 1), 1, 0);
    }
  }

  if (arg0 == 0) {
    u8* tail0 = D_1F800044;
    func_801D2C34(*(s16*)(tail0 + 0x10), 0xB3, 0, 1);
  } else {
    u8* tail1 = D_1F800044;
    func_801D2C34(*(s16*)(tail1 + 0x10), 0xB3, 1, 1);
  }
}
