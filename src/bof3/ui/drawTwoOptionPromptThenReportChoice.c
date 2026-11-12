#include "bof3/ui/shop00_internal.h"

/* @source 0x801DA678
 * @behavior shop UI two-option prompt helper: submits the prompt texture row
 *           0x1C at row 0x16 through func_8014F800 with func_801502D0(0xF) as
 *           the fifth argument and 0 and 0xFF as the middle pair, places the
 *           option cursor through func_801647C4 at the masked x
 *           ((0x1F - D_8014865B * 4) * 8 + 0x1B) with the y constant 0x18, and
 *           then reads the shared pad halfword D_80145AA8 once for the three
 *           tests below. While that pad state shares a bit with the cancel mask
 *           D_80145AC4 the helper plays cue 0x106, clears the work-block byte
 *           D_8014865B and reports a choice; while it instead shares a bit with
 *           the confirm mask D_80145AC2 the helper plays cue 0x104 when the
 *           work-block byte is set and cue 0x106 when it is clear, and reports
 *           a choice; while it shares a bit with 0xA000 the helper plays cue
 *           0x101, toggles the work-block byte and reports no choice. Every
 *           other pad state reports no choice, and every return path loads the
 *           result register with the constant 1 or 0, so the result is a whole
 *           word. The work-block byte is reached through one pointer local
 *           because the original materializes its address once, before the
 *           first read, and bases the signed coordinate read, the signed test
 *           and the zero-extended xor toggle on that single register.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 drawTwoOptionPromptThenReportChoice(void) {
  u8* flag;

  func_8014F800(0x1C, 0x16, 0, 0xFF, func_801502D0(0xF));
  flag = &D_8014865B;
  func_801647C4((u16)(((0x1F - *(s8*)flag * 4) * 8) + 0x1B), 0x18, 0);
  if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
    *flag = 0;
    return 1;
  }
  if (D_80145AA8 & D_80145AC2) {
    func_8015DF18(*(s8*)flag ? 0x104 : 0x106);
    return 1;
  }
  if (D_80145AA8 & 0xA000) {
    func_8015DF18(0x101);
    *flag = *flag ^ 1;
  }
  return 0;
}
