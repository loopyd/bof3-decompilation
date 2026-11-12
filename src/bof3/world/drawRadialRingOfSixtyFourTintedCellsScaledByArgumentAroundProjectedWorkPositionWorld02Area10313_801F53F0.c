#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-record pointer cell this overlay publishes. Every read
 * re-materialises the cell (lui $vN,0x1F80 / lw $vN,68($vN)); naming the cell
 * keeps that form where one pointer local reused across both reads collapses
 * them into a single saved register.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* The one scratchpad vector at 0x1F800014 the band centre is built in. Its vx
 * lane is written through the base register the projection argument also
 * holds, while the vy and vz lanes reach their own constant halfword cells
 * below, so the base is a vector and the other two lanes are separate symbols.
 * @source 0x1F800014 @kind data
 */
extern SVECTOR D_1F800014[];

extern s16 D_1F800016; /* @source 0x1F800016 @kind data */
extern s16 D_1F800018; /* @source 0x1F800018 @kind data */

/* The projected screen pair RotTransPers writes at 0x1F800034 and the ring
 * reads back as two unsigned halfwords at +0 and +2 of one base held in a
 * callee-saved register across the whole pass loop.
 * @source 0x1F800034 @kind data
 */
extern u16 D_1F800034[];

/* Shared frame counter of the main executable; its low bit selects which of
 * the two tint bytes this frame carries.
 * @source 0x80143E6C @kind unknown
 */
extern s32 D_80143E6C;

/* Shared distance helper of the main executable: it takes the record's
 * fixed-point position pair and returns the projected distance as a signed
 * halfword, which this body halves and negates for the band depth.
 * @source 0x8015477C
 */
extern s16 func_8015477C(s32 coord_x, s32 coord_y);

/* Primitive linker of the main executable: it orders the packet at g_PrimCursor
 * by the given cell pair, links it with the given kind and advances the cursor
 * by the given size.
 * @source 0x80155A08
 */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Primitive helper of the main executable: it prepares the 0xC-class cell the
 * body just took from g_PrimCursor.
 * @source 0x8017AA30
 */
extern void func_8017AA30(void* arg0);

/* Shared rotator pair of the main executable: the samples of a 12-bit turn in
 * the fixed-point scale the >> 14 and >> 15 products below divide out. Both are
 * called with the pass phase shifted left seven.
 * @source 0x801782FC
 */
extern s32 func_801782FC(s32 arg0);

/* @source 0x801783C8
 */
extern s32 func_801783C8(s32 arg0);

/* Shared graphic-type query used for the draw-mode tpage argument.
 * @source 0x8017B2B4
 */
extern s32 GetGraphType(void);

/* @source 0x801F53F0
 * @behavior Draws the tinted ring a work-byte-2 step handler lays over the band
 *           centre. It reads the work record published through the scratchpad
 *           pointer cell 0x1F800044, builds one scratchpad vector at
 *           0x1F800014 whose x and y are the record's signed halfwords 0x36 and
 *           0x3A minus 0x80 shifted left seven and whose z is the negation of
 *           half the distance the shared func_8015477C returns for the record's
 *           position pair 0x34/0x38, and projects that vector with RotTransPers
 *           into the scratchpad pair at 0x1F800034.
 *           It then emits the pass draw mode at g_PrimCursor with dfe 0 and dtd
 *           0, taking tpage 0x2A5 when GetGraphType reads 1 or 2 and 0xB5
 *           otherwise, links that packet through func_80155A08 with kind 1 and
 *           size 0xC, and runs 64 passes over a counter-controlled loop whose
 *           phase is the counter shifted left seven. Each pass takes a fresh
 *           primitive from g_PrimCursor, prepares it with func_8017AA30, marks
 *           it semi-transparent, and writes its x and y halfwords 0x08 and 0x0A
 *           as the projected centre plus the first and second rotator samples
 *           of the phase scaled by four times the argument and shifted right 14
 *           and 15, the y halfword adding 8. Its colour bytes 0x04, 0x05 and
 *           0x06 take four times the argument flipped about 0x3F, that value
 *           doubled, and those two gated by the low bit of the shared frame
 *           counter 0x80143E6C, so the tint alternates with the frame. Each
 *           pass then links the cell through func_80155A08 with kind 2 and size
 *           0xC after re-reading the record's position pair. Takes one argument
 *           and returns nothing.
 * @status partial
 * @match 87.22
 * @residual 116 of the 133 compared instructions match (87.22%), at 540 -> 540
 *           bytes.
 *           Every constant, call site, store and the frame are reproduced; the
 *           residual is scheduling and one scratch register. The reported first
 *           mismatch is +0x0054: the original materialises the three
 *           RotTransPers and store addresses ($a0, $a1, $a2) after the
 *           halved-distance sequence that follows the distance call, where this
 *           build places them before it, and it emits the preheader's
 *           in-place `sll $s3,$s3,0x2` before the counter clear rather than
 *           after the projected-pair base is materialised. The loop tail then
 *           differs in the same way: the original keeps the projected y halfword
 *           in $a0 from just after the x store, defers the multiply to after the
 *           colour stores and fills the HILO stall with two nops, where this
 *           build keeps the accumulator in $v0/$v1 next to the colour chain.
 *           The shapes that did not move the measurement: the three orders of
 *           the x store, the y store and the colour stores; a single sample
 *           local reused for both rotator calls; the y accumulator as a named
 *           unsigned halfword read before the colour stores; the tint levels as
 *           named locals; the vector built through a local pointer versus the
 *           0x1F800014 array base; and the x accumulator as a named local.
 *           Grouping the y halfword's +8 into the product (kept here) moved the
 *           measurement from 115 to 116.
 */
void drawRadialRingOfSixtyFourTintedCellsScaledByArgumentAroundProjectedWorkPositionWorld02Area10313_801F53F0(
    s32 arg0) {
  u8*     record;
  SVECTOR* vec;
  u8*     prim;
  s32     scale;
  s32     cy;
  s32     phase;
  s32     tpage;
  s32     i;
  s16     distance;
  s32     xSpan;
  s32     ySpan;
  s32     edge;
  long    p;
  long    flag;

  record = D_1F800044;
  xSpan = (*(s16*)(record + 0x36) - 0x80) << 7;
  ySpan = (*(s16*)(record + 0x3A) - 0x80) << 7;
  distance = func_8015477C(*(s32*)(record + 0x34), *(s32*)(record + 0x38));
  edge = -(distance / 2);
  vec = D_1F800014;
  vec->vx = xSpan;
  D_1F800016 = ySpan;
  D_1F800018 = edge;
  RotTransPers(vec, (long*)D_1F800034, &p, &flag);

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0x2A5;
  } else {
    tpage = 0xB5;
  }
  scale = arg0 << 2;
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
  func_80155A08(*(s32*)(D_1F800044 + 0x34), *(s32*)(D_1F800044 + 0x38), 1, 0xC);
  i = 0;
  do {
    prim = g_PrimCursor;
    func_8017AA30(prim);
    SetSemiTrans(prim, 1);
    phase = i << 7;
    *(u16*)(prim + 8) =
        D_1F800034[0] + ((func_801782FC(phase) * scale) >> 14);
    cy = func_801783C8(phase);
    prim[4] = (scale ^ 0x3F) << 1;
    prim[5] = ((scale ^ 0x3F) << 1) * (D_80143E6C & 1);
    prim[6] = (scale ^ 0x3F) * (D_80143E6C & 1);
    *(u16*)(prim + 0xA) = D_1F800034[1] + (((cy * scale) >> 15) + 8);
    func_80155A08(*(s32*)(D_1F800044 + 0x34), *(s32*)(D_1F800044 + 0x38), 2,
                  0xC);
    i++;
  } while (i < 0x40);
}
