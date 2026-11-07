#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044. The original
 * re-materialises the cell address for each of the five loads this body makes
 * (lui $vN,0x1F80 / lw $vN,0x44($vN)) instead of keeping one base live, and the
 * first of them fills a local that survives two stores, so the cell is bound by
 * name rather than through the constant-address accessor. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* Scratchpad radius cell at 0x1F800000, the cell the family's radial emitters
 * use as the scaling radius (scenario/emitRadialTranslucentQuads derives it the
 * same way). The original builds this address as a lui/%hi plus addiu/%lo pair
 * and keeps it in $s2 across the loop, so it is taken as the address of a named
 * cell rather than as an integer constant, whose zero low half would drop the
 * addiu. Its two published words at 0x1F800008 and 0x1F80000C are addressed by
 * literal constants instead, because the original stores them as
 * `lui $at,0x1F80` plus the offset, the absolute-constant form. */
extern s32 D_1F800000; /* @source 0x1F800000 @kind data */

/* Shared battle-kind halfword read by the sibling battle lifts (0x27 and 0xA3
 * in resolveKindResultCodeBmagicMagic03903_801EEF3C); the original reloads it
 * on every iteration with `lui` plus `lhu`, so it stays a named cell. */
extern u16 D_801463C0; /* @source 0x801463C0 @kind unknown */

/* Shared grid-cell primitive linker: takes the cell origin pair, the kind and
 * the size class, and is the same helper scenario/drawLinkedRecordCursorBeam
 * uses for its 0xC-class cells. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared primitive helpers: func_8017AA30 prepares the primitive the body just
 * took from g_PrimCursor (the sibling lifts call it on a fresh cursor primitive
 * right before SetSemiTrans), and func_801782FC/func_801783C8 return the sine
 * and cosine of a 12-bit phase in the same fixed-point scale as the >> 12
 * products below. */
void func_8017AA30(void *arg0);
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

s32 GetGraphType(void);

/* @source 0x801EF898
 * @behavior Per-frame overlay drawing step, the last of the four calls
 *           dispatchWorkByte2ThenCallUpdatePassesBmagicMagic05003_801EEF4C
 *           makes once the work record is active, and reached by no `jal`
 *           elsewhere in the payload. It first publishes three scratchpad
 *           values from the work object read through the pointer cell at
 *           0x1F800044: the radius cell at 0x1F800000 takes three times the
 *           object's unsigned byte 0x0A, the word cell at 0x1F800008 takes six
 *           times its unsigned byte 0x09, and the word cell at 0x1F80000C takes
 *           that same byte. It then runs sixteen iterations of a
 *           counter-controlled loop with phase `i << 8`, which spans the whole
 *           12-bit turn across the ring it draws. Each iteration re-reads the
 *           graph type twice to pick the draw-mode tpage argument (0xA5 when
 *           GetGraphType returns 1, 0xA5 again when it returns 2 and 0x35
 *           otherwise), passes it to SetDrawMode with dfe 0 and dtd 1, and
 *           links a kind-2, 0xC-class grid-cell primitive at the work object's
 *           cursor pair 0x34/0x38 through func_80155A08. It takes the primitive
 *           cursor g_PrimCursor as the primitive it fills, seeds it with
 *           func_8017AA30 and marks it semi-transparent, and writes the object's
 *           screen position at halfwords 0x2E/0x30 added to the sine and cosine
 *           of the phase scaled by the radius cell and shifted right 12 into the
 *           primitive's x and y halfwords at 0x08/0x0A. The primitive's colour
 *           bytes 0x04/0x05/0x06 take the two published tint words, with the
 *           channel order depending on the shared battle-kind halfword at
 *           0x801463C0: when it reads 0x32, green repeats the 0x1F800008 value
 *           and blue takes 0x1F80000C, while every other kind puts the
 *           0x1F80000C value in green and the 0x1F800008 value in blue. The
 *           iteration closes by linking the same kind-2, 0xC-class cell again
 *           and incrementing the counter. Every one of the two published tint
 *           words is re-read from the radius base at 0x1F800000 plus 0x08/0x0C
 *           before each colour store, because the byte store in between may
 *           alias them. Takes no arguments, returns nothing, keeps a 0x30-byte
 *           frame for $ra, $s3, $s2, $s1 and $s0 plus the fifth draw-mode
 *           argument at 0x10, and writes no state outside the two scratchpad
 *           words, the radius cell and the primitives it emits. The two
 *           pointer locals are the original's: the publish stores through the
 *           cell it takes at `lui $a1` plus `addiu $a1,0`, and the copy
 *           `move $s2,$a1` into the loop base follows the counter clear, so
 *           one pointer cannot serve both - a single local makes the compiler
 *           hold $s2 from the store on and drop that copy. The draw-mode test
 *           is one disjunction with a single 0xA5 assignment: the original
 *           reuses one assignment block for both graphic types and jumps to the
 *           shared join with `li $a3,0xA5` in the branch delay slot, where an
 *           if/else-if chain emits the merged compare form instead. No payload
 *           under out/binaries/emi/bmagic holds this 504-byte body, so the
 *           clone lever is ineligible; the shapes follow
 *           scenario/emitRadialTranslucentQuads, the exact sibling that
 *           publishes the same radius cell and fills the same cursor with the
 *           same sine and cosine expressions.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawRadialRingOfSixteenPrimitivesTintedByWorkByte9SwappingChannelOrderAtBattleKind50BmagicMagic05003_801EF898(
    void) {
  u8 *work;
  u8 *primitive;
  s32 *radius;
  s32 *cell;
  s32 phase;
  s32 graph_value;
  s32 i;

  work = D_1F800044;
  cell = &D_1F800000;
  *cell = work[0xA] * 3;
  *(s32 *)0x1F800008 = work[9] * 6;
  *(s32 *)0x1F80000C = work[9];

  i = 0;
  radius = cell;
  do {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      graph_value = 0xA5;
    } else {
      graph_value = 0x35;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph_value, 0);
    func_80155A08(*(s32 *)(D_1F800044 + 0x34), *(s32 *)(D_1F800044 + 0x38), 2,
                  0xC);

    primitive = g_PrimCursor;
    func_8017AA30(primitive);
    SetSemiTrans(primitive, 1);

    phase = i << 8;
    *(u16 *)(primitive + 8) = *(u16 *)(D_1F800044 + 0x2E) +
        ((func_801782FC(phase) * *radius) >> 12);
    *(u16 *)(primitive + 0xA) = *(u16 *)(D_1F800044 + 0x30) +
        ((func_801783C8(phase) * *radius) >> 12);

    if (D_801463C0 == 0x32) {
      primitive[4] = radius[2];
      primitive[5] = radius[2];
      primitive[6] = radius[3];
    } else {
      primitive[4] = radius[2];
      primitive[5] = radius[3];
      primitive[6] = radius[2];
    }

    func_80155A08(*(s32 *)(D_1F800044 + 0x34), *(s32 *)(D_1F800044 + 0x38), 2,
                  0xC);
    i++;
  } while (i < 0x10);
}
