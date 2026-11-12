#include "bof3/world/area03004_internal.h"

/* @source 0x801DCDB8
 * @behavior AREA030 scratch work-record handler (entry 1 of the 0x801E22F0
 * handler table dispatched through scratch byte 4): it first decodes the shared
 * pad state D_80145AA8 masked by 0x5000 through func_8014E640 and submits the
 * fixed func_8014F800 panel texture row (0x1D, 0x14, 0, 0xFF, 0x80010000 + the
 * main-executable halfword D_800100A4 while bit 0 of the shared panel flag byte
 * D_80144287 is set, else D_800100A2).
 * While the pad state shares a bit with the cancel mask D_80145AC4 it plays cue
 * 0x106, publishes 0xFFFF into the main-executable halfword D_801442AC, sets bit
 * 0x80 of the panel flag byte, decrements byte 3 of the work record published at
 * the scratchpad cursor 0x1F800044 and clears that record's dispatch byte 4,
 * then returns.
 * While it shares a bit with the confirm mask D_80145AC2 it plays cue 0x103,
 * sets bit 2 of the panel flag byte and advances dispatch byte 4, which selects
 * entry 2 of the same table. While the decoded direction carries the 0x5000 bits
 * it plays cue 0x100, toggles bit 0 of the panel flag byte, toggles the shared
 * state byte D_80144281 against 7 and steps dispatch byte 4 back to entry 0.
 * Unless bit 0x80 of the panel flag byte is set, it then publishes into
 * D_801442AC the selected record index byte D_80145026 (flag bit 0 set) or
 * D_80145028 (flag bit 0 clear) plus four, or 0xFFFF while that index byte is
 * zero, and finally submits func_801647C4 with (0x1C, flag bit 0 scaled by 16
 * plus 0x58, 0).
 * @status partial
 * @match 99.25
 * @residual The cancel arm's store of 0xFFFF into D_801442AC is emitted after
 * the materialisation of the D_80144287 base in the build, where the original
 * issues the constant first (its ori v0,zero,0xFFFF at 0xC1FC, the base's
 * lui/addiu at 0xC200/0xC204). Body size is the original's 536 bytes and 133 of
 * 134 instructions match; the first difference is original index 17, the
 * constant's position. Six candidate shapes (pointer assignment before the
 * constant, work pointer first, split read-modify-write through a temporary, a
 * u16 and an s32 value local, |= and cast spellings) all measured the same
 * 133/134 at 536 bytes with that first difference, so the position is not a
 * statement-order lever for this body.
 */
void publishSelectedRecordValueAndStepWorkHandlerOnPadWorld00Area03004_801DCDB8(void) {
  u16* pad;
  u8* work;
  u8* flag;
  u8 index;
  s32 direction;

  pad = &D_80145AA8;
  direction = func_8014E640(*pad & 0x5000);
  if (*pad & D_80145AC4) {
    func_8015DF18(0x106);
    D_801442AC = 0xFFFF;
    work = D_1F800044;
    flag = &D_80144287;
    *flag = *flag | 0x80;
    work[3] = work[3] - 1;
    D_1F800044[4] = 0;
    return;
  }
  if (*pad & D_80145AC2) {
    func_8015DF18(0x103);
    work = D_1F800044;
    flag = &D_80144287;
    *flag = *flag | 2;
    work[4] = work[4] + 1;
  } else if (direction & 0x5000) {
    func_8015DF18(0x100);
    flag = &D_80144287;
    *flag = *flag ^ 1;
    D_80144281 ^= 7;
    D_1F800044[4]--;
  }
  if ((D_80144287 & 1u) == 0u) {
    func_8014F800(0x1Du, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100A2);
  } else {
    func_8014F800(0x1Du, 0x14, 0, 0xFFu, 0x80010000u + (u32)D_800100A4);
  }
  if ((D_80144287 & 0x80) == 0) {
    if ((D_80144287 & 1u) == 0u) {
      index = D_80145028;
      D_801442AC = index ? index + 4 : 0xFFFF;
    } else {
      index = D_80145026;
      D_801442AC = index ? index + 4 : 0xFFFF;
    }
  }
  func_801647C4(0x1Cu, (u16)(((u32)(D_80144287 & 1u) << 4) + 0x58u), 0);
}
