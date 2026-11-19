#include "bof3/bof3.h"

void func_801F358C(u8* quad);
void clearWorkRecordCountdownsAt800E4800World01Area06714_801F39AC(void);

extern u8* volatile D_1F800044;

/* @source 0x801F2E98
 * @behavior State-3 step handler of this overlay work-byte-1 dispatcher (its
 *           pointer is word 3 of the handler table at 0x801F4624): it reads the
 *           scratchpad work record published at pointer cell 0x1F800044, steps
 *           that record's halfword at +0x10 up by 1 and clamps it to 0x40,
 *           grows the halfword at +0x12 by 0x40 and redraws the quad at +0xC
 *           through the overlay draw helper func_801F358C, then decrements the
 *           record countdown byte at +9. Once that byte reaches zero it clears
 *           the +0xC countdown of every 0xE-stride record at 0x800E4800 through
 *           clearWorkRecordCountdownsAt800E4800World01Area06714_801F39AC,
 *           zeroes the halfword at +0x14, clears its own +9 byte and advances
 *           the state byte at +1. The record pointer is re-read from the cell
 *           after the draw call and again on the advancing path, matching the
 *           three separate lui/lw pairs the original keeps, while the +0x14
 *           halfword is written through the pointer captured at entry. Takes no
 *           arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkHalfword10By1AtLimit40GrowHalfword12By40ThenClearWorkRecordCountdownsAndAdvanceWorkByte1World01Area06714_801F2E98(
    void) {
  u8* work;
  u8* quad;
  u8* count_work;
  u8* exit_work;
  s16 level;
  u8 count;

  work = D_1F800044;
  quad = work + 0xCu;
  level = (s16)(FIELD_REF(u16, work, 0x10u) + 1);
  FIELD_REF(u16, work, 0x10u) = level;
  if (level >= 0x41) {
    FIELD_REF(u16, work, 0x10u) = 0x40;
  }
  FIELD_REF(u16, work, 0x12u) += 0x40;
  func_801F358C(quad);
  count_work = D_1F800044;
  count = (u8)(count_work[9] - 1);
  count_work[9] = count;
  if (count == 0) {
    clearWorkRecordCountdownsAt800E4800World01Area06714_801F39AC();
    exit_work = D_1F800044;
    FIELD_REF(u16, work, 0x14u) = 0;
    exit_work[9] = 0;
    D_1F800044[1]++;
  }
}
