#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Work-record pointer cell at 0x801F21F0, the last word of this payload. The
 * overlay frame loop at 0x801EEC28 publishes the selected 0x2C-byte bmagic work
 * record into it and the caller at 0x801EF768 reads it before calling here, so
 * this body binds the cell by name and lets every use re-materialise the
 * address (`lui` plus `lw` after each `jal` and again inside the loop). The cell
 * sits inside the payload's trailing row, which no map row names, so the
 * reference binds through the target's generated undefined-symbol table. */
extern u8 *D_801F21F0; /* @source 0x801F21F0 @kind data */

/* Scratchpad halfword cell at 0x1F800008, the low cell of the tint triplet this
 * body publishes. The original forms its address as one `lui $a1` plus
 * `addiu $a1` pair and keeps it live across the loop, because the loop reads the
 * whole 0x1F800000-0x1F80000C block through offsets of this base: the centre
 * cells 0x1F800000/0x1F800002 at -8 and -6, the phase cell 0x1F800006 at -2, and
 * the triplet 0x1F800008/0x1F80000A/0x1F80000C at 0, 2 and 4. The two higher
 * triplet cells are published through literal constants instead, the absolute
 * `lui $at` plus `sh` form the compiler emits for a direct fixed-cell store. */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Shared grid-cell primitive linker: it resolves the (x, y) cell through the
 * shared cell-size pair, links the primitive at g_PrimCursor with the given kind
 * and advances the cursor by the given size, which is why the body re-reads
 * g_PrimCursor at the top of every iteration. This body links a 0xC-byte
 * draw-mode packet and then one 0x20-byte packet per triangle. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared primitive seed helper. The payload holds no other reference to
 * 0x8017AAC8, and the target's generated undefined-function table carries the
 * address, so no map row is owed. The body calls it with the primitive it has
 * just read from g_PrimCursor, immediately before SetSemiTrans. */
extern void func_8017AAC8(void *arg0); /* @source 0x8017AAC8 */

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` products below. Each triangle takes the
 * 0x801783C8 sample for its x vertices and the 0x801782FC sample for its y
 * vertices. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EFE2C
 * @behavior Battle-magic drawing step. Its only in-image route is the draw
 *           branch of the dispatcher at 0x801EF768, which calls it with the low
 *           16 bits of the shared frame counter at 0x80143E6C halved plus four
 *           as the first argument and with the work record unsigned byte 0x06
 *           plus half of itself as the second, so the burst it draws runs at
 *           one and a half times the radius of the neighbour at 0x801EFBF8. It
 *           first publishes the work record halfwords 0x20 and 0x22 reached
 *           through the pointer cell at 0x801F21F0 into the scratchpad cells
 *           0x1F800000 and 0x1F800002, the shared centre of the burst, and then
 *           six times the record unsigned byte 0x05 into the scratchpad halfword
 *           cells 0x1F800008, 0x1F80000A and 0x1F80000C, the tint triplet. The
 *           draw-mode tpage argument is 0xA5 when GetGraphType returns 1 or 2
 *           and 0x35 otherwise; SetDrawMode is seeded at g_PrimCursor with dfe 0
 *           and dtd 1, and the 0xC-byte draw-mode packet is linked at the record
 *           words 0x14 and 0x18 with kind 2. The body then runs four iterations
 *           from the first argument masked to 16 bits, in steps of 8, which
 *           place the low five bits of the counter shifted left 7 into the
 *           scratchpad phase cell at 0x1F800006 and so step the phase by a
 *           quarter turn each time. Every iteration takes a fresh g_PrimCursor
 *           primitive, seeds it through func_8017AAC8 and marks it
 *           semi-transparent, copies the published centre into the primitive
 *           halfwords 0x08 and 0x0A, adds the func_801783C8 and func_801782FC
 *           samples of the re-read phase cell scaled by half the second argument
 *           and shifted right 12 into the halfwords 0x10 and 0x12, adds the same
 *           two samples scaled by the whole second argument and shifted right 12
 *           into the halfwords 0x18 and 0x1A, writes 1 into primitive bytes
 *           0x04, 0x05 and 0x06, the tint triplet into bytes 0x0C, 0x0D and
 *           0x0E, 1 into bytes 0x14, 0x15 and 0x16, and links the 0x20-byte
 *           packet at the record words 0x14 and 0x18 with kind 2. So each
 *           iteration emits one triangle running from the shared centre through
 *           the half-radius vertex to the full-radius vertex, and the four
 *           phases form the rotated burst. Takes two integer arguments, returns
 *           nothing, keeps a 0x40-byte frame for $ra, all eight saved registers
 *           and the fifth draw-mode argument at 0x10, and writes no state
 *           outside the scratchpad cells and the primitives it emits.
 * @status partial
 * @match 93.98
 * @residual 156 of 166 instructions match and the build is 664 bytes, equal to
 *           the original's 664 bytes. The whole 68-instruction loop body, the
 *           draw-mode arm, the tint publication, the preheader pointer bases and
 *           the 0x40-byte frame match instruction for instruction. What remains
 *           is one compiler-pass class and the allocation that follows from it.
 *           The original keeps the raw first argument in $s7 and recomputes
 *           (arg0 & 0xFFFF) + 0x20 inside the loop's exit test as `andi` plus
 *           `addiu` plus `slt`, so its pre-test folds to `li $v0,1` plus `beqz`
 *           once combine sees that triple adjacent. This build's loop pass moves
 *           that invariant instead: the pre-test carries the three-instruction
 *           `andi`/`addiu`/`slt` triple and the exit test is one `slt` against
 *           the moved value, which also leaves the raw argument in $s0 and the
 *           scale in $s3 rather than $s7 and $s0. Six clean-C spellings of this
 *           one class measured the same hoist (if plus do-while, while, for,
 *           bare do-while, a u16 counter, and the reversed and constant-first
 *           comparison), and none of them changes the exit test. The span-equal
 *           twin at emi/bmagic/magic070/03 0x801EFA44, whose lift recorded the
 *           same class at 87.35 percent, names it a source-shape-unreachable
 *           loop-pass move after its own `-da` dump work.
 */
void drawFourRotatedTrianglesFromPublishedCenterWithHalfAndFullRadiusTintedByWorkByte5BmagicMagic07303_801EFE2C(
    s32 arg0, s32 arg1) {
  u8 *work;
  u8 *record;
  u8 *primitive;
  u16 *tint;
  u16 *center;
  s16 *phase;
  s32 scale;
  s32 half;
  s32 tpage;
  s32 i;

  work = D_801F21F0;
  *(u16 *)0x1F800000 = *(u16 *)(work + 0x20);
  *(u16 *)0x1F800002 = *(u16 *)(work + 0x22);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }

  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(*(s32 *)(D_801F21F0 + 0x14), *(s32 *)(D_801F21F0 + 0x18), 2,
                0xC);

  record = D_801F21F0;
  tint = &D_1F800008;
  *tint = record[5] * 6;
  *(u16 *)0x1F80000A = record[5] * 6;
  *(u16 *)0x1F80000C = record[5] * 6;

  i = arg0;
  i &= 0xFFFF;
  if (i < (arg0 & 0xFFFF) + 0x20) {
    phase = (s16 *)tint - 1;
    center = tint - 4;
    scale = arg1 & 0xFFFF;
    half = (u32)scale >> 1;

    /* The guard holds the zero-iteration case and the original's own top test
     * folds away, because its bound is recomputed inside the loop; the do form
     * below is the rotated loop the original emits. */
    do {
      primitive = g_PrimCursor;
      func_8017AAC8(primitive);
      SetSemiTrans(primitive, 1);

      *phase = (i & 0x1F) << 7;
      *(u16 *)(primitive + 8) = ((u16 *)phase)[-3];
      *(u16 *)(primitive + 0xA) = ((u16 *)phase)[-2];
      *(u16 *)(primitive + 0x10) =
          ((u16 *)phase)[-3] + ((func_801783C8(*phase) * half) >> 12);
      *(u16 *)(primitive + 0x12) =
          ((u16 *)phase)[-2] + ((func_801782FC(*phase) * half) >> 12);
      *(u16 *)(primitive + 0x18) =
          ((u16 *)phase)[-3] + ((func_801783C8(*phase) * scale) >> 12);
      *(u16 *)(primitive + 0x1A) =
          ((u16 *)phase)[-2] + ((func_801782FC(*phase) * scale) >> 12);

      primitive[4] = 1;
      primitive[5] = 1;
      primitive[6] = 1;
      primitive[0xC] = center[4];
      primitive[0xD] = center[5];
      primitive[0xE] = center[6];
      primitive[0x14] = 1;
      primitive[0x15] = 1;
      primitive[0x16] = 1;

      func_80155A08(*(s32 *)(D_801F21F0 + 0x14), *(s32 *)(D_801F21F0 + 0x18), 2,
                    0x20);
      i += 8;
    } while (i < (arg0 & 0xFFFF) + 0x20);
  }
}
