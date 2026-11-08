#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Work-record pointer cell at 0x801F17B4 in this overlay's trailing read-only
 * block. It holds the 0x2C-byte work record the overlay frame loop at
 * 0x801EEC28 publishes the selected entry of the 0x80-entry array based at
 * 0x801F01B4 into. The cell sits inside the reviewed trailing `textbin` row
 * labelled 0x801F00BC, which no map row names, so the reference binds through
 * the target's generated undefined-symbol table. The opening pair of halfword
 * reads shares one load, while every access group that follows a `jal`
 * re-materializes the cell address (`lui $vN,0x1F80` plus `lw $vN,0x17B4($vN)`),
 * so the cell is bound by name rather than through a live local. */
extern u8 *D_801F17B4; /* @source 0x801F17B4 @kind data */

/* Scratchpad halfword cell at 0x1F800008, the low cell of the three-word tint
 * triplet this body publishes. The original forms its address as one `lui $a1`
 * plus `addiu $a1` pair and keeps it live across the loop, because the loop
 * reads the whole 0x1F800000-0x1F80000C block through offsets of this base: the
 * centre cells 0x1F800000/0x1F800002 at -8 and -6, the phase cell 0x1F800006 at
 * -2, and the triplet 0x1F800008/0x1F80000A/0x1F80000C at 0, 2 and 4. The two
 * higher triplet cells are published through literal constants instead, the
 * absolute `lui $at` plus `sh` form the compiler emits for a direct
 * fixed-address store. */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Shared grid-cell primitive linker: it resolves the (x, y) cell through the
 * shared cell-size pair at 0x80159320/0x80159322, links the primitive at
 * g_PrimCursor with the given kind and advances the cursor by the given size,
 * which is why the body re-reads g_PrimCursor at the top of every iteration.
 * This body links a 0xC-byte draw-mode packet and then one 0x20-byte packet per
 * triangle. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared primitive seed helper. This payload holds no other reference to
 * 0x8017AAC8, so the prototype carries the address itself. It sits between the
 * primitive cursor read and SetSemiTrans, the pair position the exact sibling
 * drawRadialRingOfSixteenPrimitivesTintedByWorkByte9SwappingChannelOrderAt
 * BattleKind50BmagicMagic05003_801EF898 gives its own seed helper. The record
 * of the neighbouring body at 0x801EF810 names the same role at 0x8017AA94. */
extern void func_8017AAC8(void *arg0); /* @source 0x8017AAC8 */

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` products below. Each triangle takes the
 * 0x801783C8 sample for its x vertices and the 0x801782FC sample for its y
 * vertices. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* @source 0x801EFA44
 * @behavior Battle-magic drawing step with no caller in the payload: no `jal`,
 *           no pointer word and no table entry in any extracted binary names
 *           0x801EFA44, so the overlay loader entry is its only route and both
 *           argument roles stay structural. It first publishes the work record
 *           halfwords 0x20 and 0x22 reached through the pointer cell at
 *           0x801F17B4 into the scratchpad cells 0x1F800000 and 0x1F800002,
 *           the shared centre of the burst, and then six times the record
 *           unsigned byte 0x05 into the scratchpad halfword cells 0x1F800008,
 *           0x1F80000A and 0x1F80000C, the tint triplet. The draw-mode tpage
 *           argument is 0xA5 when GetGraphType returns 1 or 2 and 0x35
 *           otherwise; SetDrawMode is seeded at g_PrimCursor with dfe 0 and dtd
 *           1, and the 0xC-byte draw-mode packet is linked at the record words
 *           0x14 and 0x18 with kind 2. The body then runs four iterations from
 *           the first argument masked to 16 bits, in steps of 8, which place
 *           the low five bits of the counter shifted left 7 into the
 *           scratchpad phase cell at 0x1F800006 and so step the phase by a
 *           quarter turn each time. Every iteration takes a fresh g_PrimCursor
 *           primitive, seeds it through func_8017AAC8 and marks it
 *           semi-transparent, copies the published centre into the primitive
 *           halfwords 0x08 and 0x0A, adds the func_801783C8 and func_801782FC
 *           samples of the re-read phase cell scaled by half the second
 *           argument and shifted right 12 into the halfwords 0x10 and 0x12,
 *           adds the same two samples scaled by the whole second argument and
 *           shifted right 12 into the halfwords 0x18 and 0x1A, writes 1 into
 *           primitive bytes 0x04, 0x05 and 0x06, the tint triplet into bytes
 *           0x0C, 0x0D and 0x0E, 1 into bytes 0x14, 0x15 and 0x16, and links
 *           the 0x20-byte packet at the record words 0x14 and 0x18 with kind 2.
 *           So each iteration emits one triangle running from the shared centre
 *           through the half-radius vertex to the full-radius vertex, and the
 *           four phases form the rotated burst. Takes two integer arguments,
 *           returns nothing, keeps a 0x40-byte frame for $ra, all eight saved
 *           registers and the fifth draw-mode argument at 0x10, and writes no
 *           state outside the scratchpad cells and the primitives it emits.
 * @status partial
 * @match 87.35
 * @residual 145 of 166 instructions match; the build is 656 bytes against 664
 *           original bytes. The loop body, the draw-mode arm and the tint
 *           publication match instruction for instruction. Three classes
 *           remain. The entry loads the work-record cell into $v0 and the
 *           halfword value into $v1, while the original holds the cell in $v1
 *           and the loaded value in $v0; the pair is the same shape and only
 *           the register pair differs. The original keeps the raw first
 *           argument in $s7 and recomputes (arg0 & 0xFFFF) + 0x20 inside the
 *           loop's exit test as `andi` plus `addiu` plus `slt`, and its folded
 *           pre-test reads `li $v0,1` plus `beqz`; this build's loop pass moves
 *           that invariant instead, so the exit test is one `slt` against
 *           $s7 and the pre-test compares $s2 with the hoisted bound, which
 *           also hands the tint base to $s7 and the raw argument to $s0. The
 *           cc1 `-da` dumps show the owning profile's loop pass reporting
 *           `savings 2` for the `andi` and `cond forces` `savings 1` for the
 *           `addiu` and moving them for every clean-C spelling measured here
 *           (while, do-while, for, if plus do-while, while/break, u16 first
 *           argument), so the recomputation is unreachable from the source
 *           shape alone. Finally the tint base, the centre and phase bases and
 *           the masked scale are scheduled into the preheader in a different
 *           order than the original's, which follows the same hoist. The
 *           exact-shape sibling 0x801EF810 in this overlay plateaus in the
 *           same allocation class at 89.36 percent.
 */
void drawFourRotatedTrianglesFromPublishedCenterWithHalfAndFullRadiusTintedByWorkByte5BmagicMagic07003_801EFA44(
    s32 arg0, s32 arg1) {
  u16 *tint;
  u16 *center;
  s16 *phase;
  u8 *primitive;
  s32 scale;
  s32 half;
  u16 value;
  s32 tpage;
  s32 i;

  value = ((u16 *)D_801F17B4)[0x10];
  *(u16 *)0x1F800000 = value;
  value = ((u16 *)D_801F17B4)[0x11];
  *(u16 *)0x1F800002 = value;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(*(s32 *)(D_801F17B4 + 0x14), *(s32 *)(D_801F17B4 + 0x18), 2,
                0xC);

  tint = &D_1F800008;
  *tint = ((u8 *)D_801F17B4)[5] * 6;
  *(u16 *)0x1F80000A = ((u8 *)D_801F17B4)[5] * 6;
  *(u16 *)0x1F80000C = ((u8 *)D_801F17B4)[5] * 6;

  center = tint - 4;
  phase = (s16 *)tint - 1;
  scale = arg1 & 0xFFFF;
  half = (u32)(arg1 & 0xFFFF) >> 1;
  i = arg0 & 0xFFFF;
  while (i < (arg0 & 0xFFFF) + 0x20) {
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

    func_80155A08(*(s32 *)(D_801F17B4 + 0x14), *(s32 *)(D_801F17B4 + 0x18), 2,
                  0x20);
    i += 8;
  }
}
