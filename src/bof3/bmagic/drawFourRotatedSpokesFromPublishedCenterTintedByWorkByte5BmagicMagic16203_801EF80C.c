#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Record pointer cell at 0x801F17B0. The overlay frame loop at 0x801EEC28
 * publishes the current work record into this cell, and the dispatcher at
 * 0x801EF37C re-reads it on every access, so the body binds the cell by name
 * and re-reads it at each of the four points the original re-materialises the
 * address (`lui` plus `lw` after the first `jal` and again inside the loop).
 * The cell sits in this overlay's own trailing data block, so the reference
 * binds through the target's generated undefined-symbol table. */
extern u8 *D_801F17B0; /* @source 0x801F17B0 @kind data */

/* Scratchpad halfword cell at 0x1F800008, the low cell of the tint triplet
 * this body publishes. The original forms its address as one `lui $a1` plus
 * `addiu $a1` pair and keeps it live across the loop, because the loop reads
 * the whole 0x1F800000-0x1F80000C block through offsets of this base: the
 * centre cells 0x1F800000/0x1F800002 at -8 and -6, the phase cell 0x1F800006
 * at -2, and the triplet 0x1F800008/0x1F80000A/0x1F80000C at 0, 2 and 4. The
 * two higher triplet cells are published through literal constants instead,
 * the absolute `lui $at` plus `sh` form the compiler emits for a direct
 * fixed-address store. */
extern u16 D_1F800008; /* @source 0x1F800008 @kind data */

/* Shared primitive seed helper. The payload holds no other reference to
 * 0x8017AA94: the generated undefined-function table carried it only because
 * the assembly this lift replaces named it, so this prototype and the target
 * map row carry the address. Its role is the one the exact sibling
 * drawRadialRingOfSixteenPrimitivesTintedByWorkByte9SwappingChannelOrderAt
 * BattleKind50BmagicMagic05003_801EF898 binds at 0x8017AA30, which the
 * compiler places after the primitive cursor read and before SetSemiTrans. */
extern void func_8017AA94(void *arg0); /* @source 0x8017AA94 */

/* Shared grid-cell primitive linker: it resolves the (x, y) cell through the
 * shared cell-size pair, links the primitive at g_PrimCursor with the given
 * kind and advances the cursor by the given size, which is why the body
 * re-reads g_PrimCursor at the top of every iteration. This body links a
 * 0xC-byte draw-mode packet and then one 0x20-byte packet per spoke. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` products below, and this body takes the
 * 0x801783C8 sample for the second vertex x and the 0x801782FC sample for its
 * y. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* Naming and shape evidence: the payload holds one byte-identical twin of
 * this 564-byte body, emi/bmagic/magic168/03 at 0x801EF80C, whose row is still
 * assembly, so the clone lever has no source to copy and the body was seeded
 * from the original bytes. The nearest exact sibling is the same drawing step
 * of emi/bmagic/magic070/03 at 0x801EF810, which named these roles first: the
 * same published centre, the same six-times-byte-0x05 tint triplet, the same
 * draw-mode tpage selection, the same four-step phase walk and the same
 * primitive fields. The name keeps that sibling spelling under this target and
 * address. */
/* @source 0x801EF80C
 * @behavior Battle-magic drawing step reached only from the draw branch of the
 *           overlay dispatcher at 0x801EF37C, which passes half of the shared
 *           frame counter at 0x80143E6C masked to 16 bits as the first
 *           argument and the work record unsigned byte 0x06 as the second. It
 *           first publishes the work record halfwords 0x20 and 0x22 reached
 *           through the pointer cell at 0x801F17B0 into the scratchpad cells
 *           0x1F800000 and 0x1F800002, the shared centre of the burst, and then
 *           six times the record unsigned byte 0x05 into the scratchpad
 *           halfword cells 0x1F800008, 0x1F80000A and 0x1F80000C, the tint
 *           triplet. The draw-mode tpage argument is 0xA5 when GetGraphType
 *           returns 1 and 0xA5 again when it returns 2, and 0x35 otherwise;
 *           SetDrawMode is seeded at g_PrimCursor with dfe 0 and dtd 1, and the
 *           0xC-byte draw-mode packet is linked at the record words 0x14 and
 *           0x18 with kind 2. The body then runs four iterations from the first
 *           argument masked to 16 bits, in steps of 8, which place the low five
 *           bits of the counter shifted left 7 into the scratchpad phase cell
 *           at 0x1F800006 and so step the phase by a quarter turn each time.
 *           Every iteration takes a fresh g_PrimCursor primitive, seeds it
 *           through func_8017AA94 and marks it semi-transparent, copies the
 *           published centre into the primitive halfwords 0x08 and 0x0A, adds
 *           the func_801783C8 and func_801782FC samples of the re-read phase
 *           cell, scaled by the second argument and shifted right 12, into the
 *           primitive halfwords 0x10 and 0x12, writes the tint triplet into
 *           primitive bytes 0x04, 0x05 and 0x06, stores 1 into bytes 0x0C, 0x0D
 *           and 0x0E, and links the 0x20-byte packet at the record words 0x14
 *           and 0x18 with kind 2. Takes two integer arguments, returns nothing,
 *           keeps a 0x38-byte frame for $ra and all seven saved registers plus
 *           the fifth draw-mode argument at 0x10, and writes no state outside
 *           the scratchpad cells and the primitives it emits.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFourRotatedSpokesFromPublishedCenterTintedByWorkByte5BmagicMagic16203_801EF80C(
    s32 arg0, s32 arg1) {
  u8 *work;
  u8 *record;
  u8 *primitive;
  u16 *tint;
  u16 *center;
  s16 *phase;
  s32 scale;
  s32 tpage;
  s32 i;

  work = D_801F17B0;
  *(u16 *)0x1F800000 = *(u16 *)(work + 0x20);
  *(u16 *)0x1F800002 = *(u16 *)(work + 0x22);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }

  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(*(s32 *)(D_801F17B0 + 0x14), *(s32 *)(D_801F17B0 + 0x18), 2,
                0xC);

  record = D_801F17B0;
  tint = &D_1F800008;
  *tint = record[5] * 6;
  *(u16 *)0x1F80000A = record[5] * 6;
  *(u16 *)0x1F80000C = record[5] * 6;

  i = arg0 & 0xFFFF;
  if (i < (arg0 & 0xFFFF) + 0x20) {
    phase = (s16 *)tint - 1;
    center = tint - 4;
    scale = arg1 & 0xFFFF;

    /* The guard holds the zero-iteration case; the do form is what the
     * original emits, with the initial compare against a freshly computed
     * bound and the loop compare against the copy of it kept in a saved
     * register. */
    do {
      primitive = g_PrimCursor;
      func_8017AA94(primitive);
      SetSemiTrans(primitive, 1);

      *phase = (i & 0x1F) << 7;
      *(u16 *)(primitive + 8) = ((u16 *)phase)[-3];
      *(u16 *)(primitive + 0xA) = ((u16 *)phase)[-2];
      *(u16 *)(primitive + 0x10) =
          ((u16 *)phase)[-3] + ((func_801783C8(*phase) * scale) >> 12);
      *(u16 *)(primitive + 0x12) =
          ((u16 *)phase)[-2] + ((func_801782FC(*phase) * scale) >> 12);

      primitive[4] = center[4];
      primitive[5] = center[5];
      primitive[6] = center[6];
      primitive[0xC] = 1;
      primitive[0xD] = 1;
      primitive[0xE] = 1;

      func_80155A08(*(s32 *)(D_801F17B0 + 0x14), *(s32 *)(D_801F17B0 + 0x18), 2,
                    0x20);
      i += 8;
    } while (i < (arg0 & 0xFFFF) + 0x20);
  }
}
