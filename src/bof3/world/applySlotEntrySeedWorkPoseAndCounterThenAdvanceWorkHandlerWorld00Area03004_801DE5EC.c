#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 work-record entry step: it applies the entry halfword at
 * 0x10 of the D_801E2AB8 scale record selected by work-record byte 0x06 through
 * func_8014DD3C, clears work bytes 0x24, 0x09 and 0x0B, sets work bytes 0x48 to
 * 2 and 0x29 to 5, seeds the record heading byte 0x92 with rand & 3 raised to
 * the scale record byte 0x1A and masked back to two bits, derives the record
 * x at 0x34 from the 0x140000 ceiling less the low nibble of a random roll
 * shifted 16, derives the record y at 0x38 from the 0x3E0000 ceiling less the
 * heading times 0xC0000 and the same random term, seeds the signed counter at
 * 0x3E with the negated scale byte times 256 less rand & 7 times 32, clamps that
 * counter against the func_8015477C distance of the seeded x/y pair (to -0x20
 * when it reads zero, otherwise up to the distance when it sits below it), then
 * sets work byte 0x08 to 4, clears the work word at 0x14, releases the entry
 * through func_8014D6B8(0) and advances the work handler byte 0x01.
 * @source 0x801DE5EC
 * @status partial
 * @match 95.14
 * @residual 137/144 instructions and an exact 576-byte body; two scheduler
 * placements remain. The scratch-cursor reload that feeds the heading mask
 * sits two instructions later than the original and pays a load-delay nop, and
 * the scale-shift of the counter lands in the rand delay slot where the
 * original keeps the negate there. Registers, statements and values are
 * otherwise identical.
 */
void applySlotEntrySeedWorkPoseAndCounterThenAdvanceWorkHandlerWorld00Area03004_801DE5EC(void) {
  u8* yWork;
  u8* callWork;
  u8* clampWork;
  u32 scale;
  u32 heading;
  s32 base;
  s32 counter;
  s16 limit;
  u16 distance;

  /* One cursor variable per reload point: the cell is volatile, so each group
   * re-reads it exactly where the original does. Collapsing the three reads
   * into one variable emits a single load and loses the whole tail. */
  func_8014DD3C(D_801E2AB8[D_1F800044[6]].entry_10);

  D_1F800044[0x24] = 0;
  D_1F800044[0x48] = 2;
  D_1F800044[0x29] = 5;
  D_1F800044[9] = 0;
  D_1F800044[0xB] = 0;

  *(u8*)((u8*)D_80146884 + 0x92) = rand() & 3;

  scale = D_801E2AB8[D_1F800044[6]].unk_1A;
  if (*(u8*)((u8*)D_80146884 + 0x92) < scale) {
    *(u8*)((u8*)D_80146884 + 0x92) = scale;
  }
  *(u8*)((u8*)D_80146884 + 0x92) &= 3;

  /* The x ceiling is materialized ahead of its rand call, so it rides in the
   * saved register the original keeps the y accumulator in. */
  base = 0x140000;
  *(s32*)(D_1F800044 + 0x34) = base - ((rand() & 0xF) << 16);

  heading = *(u8*)((u8*)D_80146884 + 0x92);
  base = 0x3E0000;
  base -= heading * 3 << 18;
  base -= (rand() & 0xF) << 16;
  yWork = D_1F800044;
  *(s32*)(yWork + 0x38) = base;

  /* Sign-extended scale byte, negated and scaled: the -base >> 16 pair is the
   * original's negate-then-shift of the byte placed in the high half. */
  base = D_801E2AB8[yWork[6]].unk_1A << 24;
  counter = (-base >> 16) - ((rand() & 7) << 5);

  callWork = D_1F800044;
  *(s16*)(callWork + 0x3E) = counter;
  distance = func_8015477C(*(s32*)(callWork + 0x34), *(s32*)(callWork + 0x38));

  clampWork = D_1F800044;
  limit = *(s16*)(clampWork + 0x3E);
  if (limit == 0) {
    *(s16*)(clampWork + 0x3E) = -0x20;
  } else if (limit < (s16)distance) {
    *(s16*)(clampWork + 0x3E) = distance;
  }

  D_1F800044[8] = 4;
  *(s32*)(D_1F800044 + 0x14) = 0;
  func_8014D6B8(0);
  D_1F800044[1]++;
}
