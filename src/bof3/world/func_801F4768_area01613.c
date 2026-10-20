#include "bof3/world/area01613_internal.h"

#include <stdlib.h>

/* @source 0x801F4768
 * @behavior On the first active frame it advances the scratch byte at offset
 * `0x02` and adds the low four bits of the shared frame counter at 0x80143E6C
 * to the scratch halfword at `0x3A`, then returns unless bit `0x04` of the
 * shared byte at 0x8014832E is set. It derives the panel layer from the
 * scratch byte at `0x0B` (`layer = value - 2`), advances the scratch scroll
 * word at `0x38` by `0x800 + (layer << 10)`, clamps the scratch halfword at
 * `0x3A` to `-8` once the shared map byte at 0x80104001 plus `8` no longer
 * exceeds it, and returns unless the scratch halfwords at `0x36`/`0x3A` are
 * within `0x1A` of the shared halfwords at 0x8014930A/0x8014930E. It then
 * projects one shared `0x28`-byte textured quad (clut `0x78CB`, tpage `0x12B`
 * for graphics types 1 and 2 and `0x5B` otherwise, flat colour `0x28`) through
 * `RotTransPers4` over the four scratchpad vectors at `0x1F800014`, whose
 * halfsize pulses by `|0x0F - (frame counter & 0x1F)|` around the projected
 * scratch halfwords at `0x34`/`0x38` scaled by `>> 9` and biased by `-0x3FC0`;
 * it closes with `func_80155560(layer | 0xBB509100, prim, 1)` and reserves the
 * primitive through `func_8014E5A0(4, 0x28)`. Finally it walks a
 * `(2 - layer) * 4 + 1` by `(2 - layer) * 4` grid: for each cell it looks the
 * two scratch halfwords at `0x36`/`0x3A` biased by `- 2 + layer` plus the cell
 * column/row up in the shared 0x801559AC lookup, and, when that returns a
 * record, re-seeds the current primitive from `g_PrimCursor`, copies the four
 * projected position words at offsets `0x08`/`0x10`/`0x18`/`0x20` of the
 * record into it and fills its texture window from the target-local byte
 * columns `D_801F51D8` (u origin), `D_801F51DC` (v origin) and `D_801F51E0`
 * (tile size) at index `layer`, scrolling v by
 * `(size * (scratch u16 at 0x38)) / 0x10000`, before queueing the cell through
 * `func_80155A08(..., 1, 0x28)` with both coordinates shifted left by `16`.
 * @status partial
 * @match 83.03
 * @residual live audit 323/389 instructions, 1556 bytes against 1556 original; the
 * first-active-frame entry block now matches byte for byte - two separate `lbu` reads
 * of the byte at `0x02`, the `0x3A` store ahead of the `0x02` store, the incremented
 * value held in a local and the entry pointer allocated to `$a1` as in the original.
 * First differs at +0x008C: the byte at `0x0B` is allocated to `$s3` instead of the
 * original's `$v1` with `layer` in `$s2`, and `0x800 + (layer << 10)` folds into
 * `value << 10` where the original emits `addiu s2,v1,-2`, `addiu v0,v0,0x800` and
 * `sll v1,s2,0xa` separately. Volatility lever measured: a function-wide volatile
 * pointee gives 307/389 at 1568 bytes, the entry-scope volatile view alone 322/389 at
 * 1556 bytes, and the `next` local the retained 323/389. Clean-C levers measured and
 * rejected: `row < span + 1` (277/391), `x`/`y` field-bias locals (297/391), explicit
 * `(s16)` casts on the bias (309/399), inner bound `span` without the guard (276/389),
 * locals declared in a different order (no change), and the entry statement order
 * `field` then increment (304/389, exact 1556 bytes). Smallest missing evidence: the
 * original's allocation of `layer` across the scroll/clamp block, which only an
 * allocator-level change would expose; the default clean-C ladder is exhausted and the
 * profile rungs (compiler-variants/permuter) are opt-in and unauthorized here. Rung 4
 * is measured negative: `lift flag-search` over 52 catalogue profiles returns no
 * exact match and leaves the -O2/-G0 baseline best, so the cap is not a
 * compiler-profile class. The review's cancellation lever is also measured negative:
 * splitting the `0x38` update into `+ 0x800` and `+ (layer << 10)` statements, with
 * and without a named `scroll` temporary, both emit 303/390 at 1560 bytes with the
 * frame one saved register smaller and the first difference moved to `+0x0008`;
 * gcc-2.7.2 reassociates the halves back into `value << 10` and only the allocation
 * changes, so both shapes were reverted to this candidate.
 */
void func_801F4768(void) {
  volatile World00Area016Scratch* first_frame;
  World00Area016Scratch* scratch;
  u32                    primitive;
  SVECTOR*               vertices;
  void*                  record;
  s32                    layer;
  s32                    span;
  s32                    row;
  s32                    column;
  s32                    half;
  s32                    x;
  s32                    y;
  s32                    tpage;
  long                   depth;
  long                   flag;

  first_frame = D_1F800044;
  if (first_frame->state_02 == 0) {
    u8 count;
    u8 next;

    count = first_frame->state_02;
    next = count + 1;
    FIELD_REF(volatile u16, first_frame, 0x3au) += D_80143E6C & 0xf;
    first_frame->state_02 = next;
  }
  if ((D_8014832E & 4) == 0) {
    return;
  }

  scratch = D_1F800044;
  layer = (s32)scratch->unk_0b - 2;
  scratch->unk_38 = scratch->unk_38 + 0x800 + (layer << 10);
  if (D_80104001 + 8 < FIELD_REF(s16, scratch, 0x3au)) {
    FIELD_REF(s16, scratch, 0x3au) = -8;
  }

  scratch = D_1F800044;
  if (abs(D_8014930A - FIELD_REF(s16, scratch, 0x36u)) >= 0x1a &&
      abs(D_8014930E - FIELD_REF(s16, scratch, 0x3au)) >= 0x1a) {
    return;
  }

  primitive = (u32)g_PrimCursor;
  row = 0;
  SetPolyFT4((POLY_FT4*)primitive);
  SetShadeTex((POLY_FT4*)primitive, 0);

  vertices = D_1F800014;
  SPAD_REF(s16, 0x18u) = -0x300;
  SPAD_REF(s16, 0x20u) = -0x300;
  SPAD_REF(s16, 0x28u) = -0x300;
  SPAD_REF(s16, 0x30u) = -0x300;

  half = ((2 - layer) * 256) + abs(0xf - (D_80143E6C & 0x1f));
  x = ((s32)D_1F800044->unk_34 >> 9) - 0x3fc0;
  y = ((s32)D_1F800044->unk_38 >> 9) - 0x3fc0;

  vertices[0].vx = x - half;
  SPAD_REF(s16, 0x16u) = y - half;
  vertices[1].vx = x + half;
  SPAD_REF(s16, 0x1eu) = y - half;
  vertices[2].vx = x - half;
  SPAD_REF(s16, 0x26u) = y + half;
  vertices[3].vx = x + half;
  SPAD_REF(s16, 0x2eu) = y + half;

  RotTransPers4(vertices, vertices + 1, vertices + 2, vertices + 3,
                (long*)(primitive + 8), (long*)(primitive + 0x10),
                (long*)(primitive + 0x18), (long*)(primitive + 0x20), &depth,
                &flag);

  func_80155560(layer | 0xbb509100u, (void*)primitive, 1);
  func_8014E5A0(4, 0x28);

  span = (2 - layer) * 4;
  for (row = 0; row <= span; row++) {
    if (span <= 0) {
      continue;
    }
    for (column = 0; column < (2 - layer) * 4; column++) {
      record = func_801559AC(
          FIELD_REF(s16, D_1F800044, 0x36u) - 2 + layer + column,
          FIELD_REF(s16, D_1F800044, 0x3au) - 2 + layer + row);
      if (record == 0) {
        continue;
      }

      primitive = (u32)g_PrimCursor;
      SetPolyFT4((POLY_FT4*)primitive);
      SetShadeTex((POLY_FT4*)primitive, 0);
      SetSemiTrans((POLY_FT4*)primitive, 1);

      *(u32*)(primitive + 8) = *(u32*)((u8*)record + 8);
      *(u32*)(primitive + 0x10) = *(u32*)((u8*)record + 0x10);
      *(u32*)(primitive + 0x18) = *(u32*)((u8*)record + 0x18);
      *(u32*)(primitive + 0x20) = *(u32*)((u8*)record + 0x20);
      *(u16*)(primitive + 0x0e) = 0x78cb;

      if (GetGraphType() == 1) {
        tpage = 0x12b;
      } else if (GetGraphType() == 2) {
        tpage = 0x12b;
      } else {
        tpage = 0x5b;
      }
      *(u16*)(primitive + 0x16) = tpage;

      *(u8*)(primitive + 4) = 0x28;
      *(u8*)(primitive + 5) = 0x28;
      *(u8*)(primitive + 6) = 0x28;

      *(u8*)(primitive + 0x0c) = D_801F51D8[layer] + column * D_801F51E0[layer];
      *(u8*)(primitive + 0x0d) = D_801F51DC[layer] +
                                 row * D_801F51E0[layer] -
                                 (D_801F51E0[layer] *
                                  FIELD_REF(u16, D_1F800044, 0x38u)) /
                                     0x10000;
      *(u8*)(primitive + 0x14) =
          D_801F51E0[layer] +
          (D_801F51D8[layer] + column * D_801F51E0[layer]);
      *(u8*)(primitive + 0x15) = D_801F51DC[layer] +
                                 row * D_801F51E0[layer] -
                                 (D_801F51E0[layer] *
                                  FIELD_REF(u16, D_1F800044, 0x38u)) /
                                     0x10000;
      *(u8*)(primitive + 0x1c) = D_801F51D8[layer] + column * D_801F51E0[layer];
      *(u8*)(primitive + 0x1d) =
          D_801F51E0[layer] +
          (D_801F51DC[layer] + row * D_801F51E0[layer] -
           (D_801F51E0[layer] * FIELD_REF(u16, D_1F800044, 0x38u)) / 0x10000);
      *(u8*)(primitive + 0x24) =
          D_801F51E0[layer] +
          (D_801F51D8[layer] + column * D_801F51E0[layer]);
      *(u8*)(primitive + 0x25) =
          D_801F51E0[layer] +
          (D_801F51DC[layer] + row * D_801F51E0[layer] -
           (D_801F51E0[layer] * FIELD_REF(u16, D_1F800044, 0x38u)) / 0x10000);

      func_80155A08(
          (FIELD_REF(s16, D_1F800044, 0x36u) - 2 + layer + column) << 16,
          (FIELD_REF(s16, D_1F800044, 0x3au) - 2 + layer + row) << 16, 1, 0x28);
    }
  }
}
