#include "bof3/world/area03004_internal.h"

/**
 * @source 0x801E03F8
 * @behavior Re-derives the movement step of the AREA030 work record published
 * at the scratchpad cursor 0x1F800044, confines it to the area's staging
 * rectangle and re-indexes the route table.
 * It first copies the route entry selected by record byte +0x08 out of the
 * global table pair 0x80181AC0 / 0x80181AC4 into the record step words at
 * +0x0C / +0x10, doubling both entries when that route index is 2 or 6.
 * The x step is then flipped so the record x at +0x34 heads back into the band
 * 0x80000..0x13FFFF (below it the step is forced non-negative, above it
 * non-positive), and the y step at +0x10 is flipped to keep the record y at
 * +0x38 inside the staging window [lo, hi] derived from the 0x80146884 byte
 * +0x92 (m, times 0xC0000) and the scale record D_801E2AB8[record byte +0x06]
 * byte +0x1B (t, times 15): hi = 0x3E0000 - m * 0xC0000 + (t * 15 << 16)
 * clamped to 0x1A0000..0x3E0000 and lo = 0x320000 - m * 0xC0000 - (t * 15 <<
 * 16) clamped to 0x110000..0x320000.
 * It finally scans the eight route entries for the first one whose step pair
 * (each word doubled when its magnitude is below 0x401) equals the record step
 * pair and stores that route index, or 8 when none matches, into record byte
 * +0x08.
 * @status partial
 * @match 29.01
 * @residual live asm-diff first=+0x0000[o0/c0]: the original opens with
 * `lui $a0,(0x1F800044 >> 16); lw $a0,..` and keeps the record cursor in $a0
 * through the first three store groups, while this source's allocator picks
 * $a2 for that pseudo, so the byte stream diverges from the first instruction
 * even though the shared instruction shape matches for the first 29
 * instructions. Remaining measured differences: 157/162 instructions, 628/648
 * bytes; the 0x19FFFF / 0x1A0000 clamp constants are materialised three times
 * here instead of once (`lui` copies land in the x-clamp branch delay slots),
 * the x-clamp `negu` lands in the jump delay slot instead of the branch delay
 * slot, and the y-band and route-scan blocks schedule two instructions
 * earlier. Clean-C only: no pins, clobbers, barriers or empty asm.
 * Established clean-C facts for this selector: the route-scan loop must carry
 * an explicit byte-offset local (a `D_80181AC0[i * 2]` index makes gcc 2.7 emit
 * hoisted address bases via `la` and loses 5 instructions), the band limits are
 * the literals 0x19FFFF / 0x10FFFF with 0x1A0000 / 0x110000 floors, and
 * `hi = 0x3E0000 - spread` + `hi += shift` (split statements) preserves the
 * original's `subu`+`addu` pair where the single expression reassociates.
 * Next untried rung: one bounded `bin/permute
 * emi/world00/area030/04@0x801E03F8 --time-limit 60` after the residual
 * allocation is classified.
 */
void func_801E03F8(void) {
  u8* work;
  s32 spread;
  s32 shift;
  s32 m;
  s32 t;
  s32 hi;
  s32 lo;
  s32 x;
  s32 y;
  s32 sx;
  s32 sy;
  s32 magnitude;
  s32 i;
  s32 offset;

  work = D_1F800044;
  *(s32*)(work + 0x0C) = D_80181AC0[work[8] * 2];
  *(s32*)(work + 0x10) = D_80181AC4[work[8] * 2];
  if (work[8] == 2 || work[8] == 6) {
    *(s32*)(work + 0x0C) = *(s32*)(work + 0x0C) * 2;
    *(s32*)(work + 0x10) = *(s32*)(work + 0x10) * 2;
  }

  work = D_1F800044;
  x = *(s32*)(work + 0x34);
  if (x <= 0x80000) {
    if (*(s32*)(work + 0x0C) < 0) {
      *(s32*)(work + 0x0C) = -*(s32*)(work + 0x0C);
    }
  } else if (x > 0x13FFFF) {
    if (*(s32*)(work + 0x0C) > 0) {
      *(s32*)(work + 0x0C) = -*(s32*)(work + 0x0C);
    }
  }

  m = *(u8*)((u8*)D_80146884 + 0x92);
  work = D_1F800044;
  t = D_801E2AB8[work[6]].unk_1B;
  spread = m * 0xC0000;
  shift = t * 15 << 16;
  hi = 0x3E0000 - spread;
  lo = 0x320000 - spread;
  hi += shift;
  lo -= shift;
  if (hi <= 0x19FFFF) {
    hi = 0x1A0000;
  } else if (hi > 0x3E0000) {
    hi = 0x3E0000;
  }
  if (lo <= 0x10FFFF) {
    lo = 0x110000;
  } else if (lo > 0x320000) {
    lo = 0x320000;
  }

  work = D_1F800044;
  y = *(s32*)(work + 0x38);
  if (y >= hi) {
    if (*(s32*)(work + 0x10) > 0) {
      *(s32*)(work + 0x10) = -*(s32*)(work + 0x10);
    }
  } else if (y <= lo) {
    if (*(s32*)(work + 0x10) < 0) {
      *(s32*)(work + 0x10) = -*(s32*)(work + 0x10);
    }
  }

  work = D_1F800044;
  offset = 0;
  for (i = 0; i < 8; i++) {
    sx = *(s32*)((u8*)D_80181AC0 + offset);
    sy = *(s32*)((u8*)D_80181AC4 + offset);
    magnitude = sx;
    if (magnitude < 0) {
      magnitude = -magnitude;
    }
    if (magnitude < 0x401) {
      sx = sx * 2;
    }
    magnitude = sy;
    if (magnitude < 0) {
      magnitude = -magnitude;
    }
    if (magnitude < 0x401) {
      sy = sy * 2;
    }
    if (*(s32*)(work + 0x0C) == sx && *(s32*)(work + 0x10) == sy) {
      break;
    }
    offset += 8;
  }

  work[8] = i;
}
