#include "bof3/bof3.h"

extern u8* g_PrimCursor; /* @source 0x8014598C @kind bss */

void func_8014E5A0(u8 arg0, u8 arg1);
void func_801AFE18(void* arg0);
void drawRedGradientBandCenteredOnWorkEntryHalfwordWithFrameParityHalfHeightScenarioSce15ef200_801D1414(
    u8* record);

/* @source 0x801D1624
 * @behavior Per-frame step of the 0x10-entry work table at 0x800E4800, the
 * six-byte-stride record table the scenario, world and battle overlays share:
 * it first arms the sprite draw mode for the table - GetTPage(0, 1, 0x380,
 * 0x100) yields the texture page, that page is handed to SetDrawMode together
 * with the primitive published at g_PrimCursor, and func_8014E5A0 queues the
 * 0x0C-byte DR_MODE packet - then fills the 0x20-byte projection scratch
 * through func_801AFE18, and walks the 0x10 entries with an unsigned byte
 * counter, stepping the entry base pointer by six. For every entry whose claim
 * byte at +0x0 reads non-zero it advances that entry's state byte at +0x2: on
 * state 0 it claims the entry, writing 4 to the entry bytes at +0x1 and +0x3,
 * its argument to the signed halfword at +0x4 and one more than the state byte
 * back to the state byte; on state 1 it increments the state byte; on state 2 it
 * subtracts 8 from the halfword at +0x4 and clears the claim byte once that
 * halfword reads negative, which releases the entry. After the switch it draws
 * every entry whose state byte is still non-zero through the in-payload helper
 * drawRedGradientBandCenteredOnWorkEntryHalfwordWithFrameParityHalfHeightScenarioSce15ef200_801D1414
 * (0x801D1414) with the entry base in $a0. It returns non-zero in the frames
 * where at least one entry claim byte read non-zero; both in-payload call sites,
 * the overlay handler run entries at 0x801D0E30 and 0x801D0EC4, pass the literal
 * 0xF0 - the halfword the arm stores - so the argument and the result are full
 * words and only the result's low byte is consumed. The 0x58-byte frame holds
 * SetDrawMode's outgoing fifth argument at +0x10, the 0x20-byte projection
 * scratch at +0x18 and the saved $ra with $s0-$s5 of the walk.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the frame-active flag has to stay an unsigned byte local. With
 * it declared s32, gcc folds the arm's `active = 1` into the switch's compare
 * constant, hands the compare delay slot to the state-1 arm and cross-jumps that
 * arm into the state-0 tail; that shape measured 81/95 at 368 bytes with
 * first=+0x008c, and the u8 local measured 95/95 at the unchanged 380 bytes.
 * Declaring the flag after the pointers, as an s32 that owns the compare
 * constant, was not what the original bytes encode.
 */
s32 stepWorkTableEntriesArmAdvanceDecayThenDrawActiveScenarioSce15ef200_801D1624(
    s32 entry_value) {
  u8 scratch[0x20];
  u8* record;
  u8 index;
  u16 tpage;
  u8 active;

  active = 0;
  tpage = GetTPage(0, 1, 0x380, 0x100);
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, tpage, 0);
  record = PSX_PTR(u8, 0x800E4800u);
  index = 0;
  func_8014E5A0(7, 0xC);
  func_801AFE18(scratch);
  do {
    if (FIELD_REF(u8, record, 0x0) != 0) {
      active = 1;
      switch (FIELD_REF(u8, record, 0x2)) {
        case 0:
          FIELD_REF(u8, record, 0x1) = 4;
          FIELD_REF(u8, record, 0x3) = 4;
          FIELD_REF(s16, record, 0x4) = entry_value;
          FIELD_REF(u8, record, 0x2) += 1;
          break;
        case 1:
          FIELD_REF(u8, record, 0x2) += 1;
          break;
        case 2:
          FIELD_REF(s16, record, 0x4) -= 8;
          if (FIELD_REF(s16, record, 0x4) < 0) {
            FIELD_REF(u8, record, 0x0) = 0;
          }
          break;
      }
      if (FIELD_REF(u8, record, 0x2) != 0) {
        drawRedGradientBandCenteredOnWorkEntryHalfwordWithFrameParityHalfHeightScenarioSce15ef200_801D1414(
            record);
      }
    }
    record += 0x6;
    index += 1;
  } while (index < 0x10);

  return active;
}
