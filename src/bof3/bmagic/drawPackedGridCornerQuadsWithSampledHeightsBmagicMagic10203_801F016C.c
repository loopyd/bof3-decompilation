#include "bof3/bof3.h"
#include "gpu/prim.h"

/* One packed grid corner of a quad record: a signed depth offset in the low
 * halfword, the signed grid row in byte 2 and the signed grid column in byte 3.
 * The drawing loop adds the corner as a whole word and keeps only the low
 * halfword of the sum, so the word view is the one it reads.
 */
typedef union {
  s32 depth;
  struct {
    s16 z;
    s8 y;
    s8 x;
  } grid;
} GridCorner;

/* One packed vertex colour of a quad record: blue in byte 0, green in byte 1,
 * red in byte 2, and the page index with the semi-transparency bit in byte 3.
 * The original reads red as a halfword, the other two channels through the
 * packed word and the page byte on its own.
 */
typedef union {
  u32 word;
  struct {
    u16 bg;
    u16 rf;
  } half;
  struct {
    u8 b;
    u8 g;
    u8 r;
    u8 flags;
  } byte;
} GridColor;

/* Record this overlay passes in. Word zero is a header: a signed halfword the
 * shared view test reads, a byte counting the record's four-byte words (the
 * count includes the header word itself) and a pad byte. Word one starts the
 * payload, a run of eight-word quad records, each four packed grid corners
 * followed by four packed vertex colours.
 */
typedef union {
  struct {
    s16 unk_00;
    u8 unk_02;
    u8 unk_03;
  } head;
  u32 word[1];
} GridQuadRecord;

/* Ordering table the emitted primitives are appended to.
 * @source 0x8014933E @kind data
 */
extern u8 D_8014933E;

/* Tests the record against the view passed in and answers zero when the caller
 * should skip the record; its roles stay structural.
 * @source 0x80155E7C @kind function
 */
extern s8 func_80155E7C(s16 arg0, s32 arg1);

/* Height sampler of this overlay's signed byte grid.
 * @source 0x801F0ADC @kind function
 */
extern s16 sampleNegatedHeightFromGridWithMirroredAxesBmagicMagic10203_801F0ADC(
    s32 px,
    s32 py);

/* @source 0x801F016C
 * @behavior Draws the translucent Gouraud quads of the packed quad record the
 * caller passes, one quad per eight-word record, with every corner depth
 * sampled from this overlay's signed byte height grid. It first tests the
 * record against the first coordinate through the shared view test at
 * 0x80155E7C and returns without touching the ordering table when that test
 * answers zero. It then scales both coordinates by 128 around 0x80 and samples
 * the height grid once at the corner one step back on the first axis, keeping
 * that first sample only for its side effect. The record's byte 2 counts its
 * own header word, so the word cursor starts at word one and the run ends when
 * it reaches that count; a record whose count is one draws no quad. For each
 * record the texture page is the top two bits of the fourth byte of the
 * record's first colour word, shifted onto the 0x225 base by seven for graph
 * type 1 or 2 and onto the 0x95 base by five otherwise. It sets the draw mode
 * on the primitive cursor with dithering on and no texture window, appends the
 * twelve-byte draw-mode packet to ordering table entry D_8014933E, and starts
 * a Gouraud quad at the cursor. Each of the four packed corners adds its
 * signed column and row offsets doubled to the scaled coordinates, samples the
 * height grid at that pair and adds the corner word, keeping the low halfword
 * as depth; the four points are then projected in the rotated order one, two,
 * three, zero straight into the quad's screen coordinate pairs one, two, three,
 * zero. Semi-transparency comes from bit 31 of the first colour word, and the
 * four colours are copied channel by channel, red taken from the halfword at
 * the third byte, green from the colour word shifted right eight and blue from
 * the colour word itself. The quad's thirty-six bytes are appended and the run
 * repeats. Afterwards it sets the draw mode again on the cursor with dithering
 * off and the plain page constant, and appends the closing twelve-byte packet.
 * The body keeps a 0x80 frame for the four projected points, the projection
 * depth and flag and the spilled second coordinate, and returns nothing.
 * @status partial
 * @match 99.50
 * @residual One instruction of 199 and four of 796 bytes differ: the original
 * saves $ra before $fp in the prologue where this build saves $fp first. The
 * body, the frame layout, the saved register set and the byte count are
 * identical, and the order of that store pair held across every shape measured
 * in this lane (corner view as union or plain struct, word cursor or index,
 * loop increment placement, parameter and local declaration order, pointer
 * types), so it stays a compile-side residual.
 */
void drawPackedGridCornerQuadsWithSampledHeightsBmagicMagic10203_801F016C(
    GridQuadRecord *rec,
    s32 x,
    s32 y) {
  SVECTOR shape[4];
  POLY_G4 *prim;
  s32 depth;
  s32 flag;
  s32 page;
  s32 i;
  s32 j;

  if (func_80155E7C(rec->head.unk_00, x) == 0) {
    return;
  }
  x = (x - 0x80) << 7;
  y = (y - 0x80) << 7;
  sampleNegatedHeightFromGridWithMirroredAxesBmagicMagic10203_801F0ADC(
      x - 0x80, y);
  i = 1;
  while (i != rec->head.unk_02) {
    if (GetGraphType() == 1 || GetGraphType() == 2) {
      page = (((u8 *)&rec->word[i])[0x13] & 3) << 7 | 0x225;
    } else {
      page = (((u8 *)&rec->word[i])[0x13] & 3) << 5 | 0x95;
    }
    SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, page, 0);
    GpuAppendPrim(D_8014933E, 0xC);
    prim = (POLY_G4 *)g_PrimCursor;
    SetPolyG4(prim);
    for (j = 0; j < 4; j++, i++) {
      shape[j].vx = x + (((GridCorner *)&rec->word[i])->grid.x * 2);
      shape[j].vy = y + (((GridCorner *)&rec->word[i])->grid.y * 2);
      shape[j].vz =
          sampleNegatedHeightFromGridWithMirroredAxesBmagicMagic10203_801F0ADC(
              shape[j].vx, shape[j].vy) +
          ((GridCorner *)&rec->word[i])->depth;
    }
    RotTransPers4(&shape[1], &shape[2], &shape[3], &shape[0],
                  (long *)&prim->x1, (long *)&prim->x2, (long *)&prim->x3,
                  (long *)&prim->x0, &depth, &flag);
    SetSemiTrans(prim, ((GridColor *)&rec->word[i])->word >> 31);
    prim->r0 = ((GridColor *)&rec->word[i])->half.rf;
    prim->g0 = ((GridColor *)&rec->word[i])->word >> 8;
    prim->b0 = ((GridColor *)&rec->word[i])->word;
    i++;
    prim->r1 = ((GridColor *)&rec->word[i])->half.rf;
    prim->g1 = ((GridColor *)&rec->word[i])->word >> 8;
    prim->b1 = ((GridColor *)&rec->word[i])->word;
    i++;
    prim->r2 = ((GridColor *)&rec->word[i])->half.rf;
    prim->g2 = ((GridColor *)&rec->word[i])->word >> 8;
    prim->b2 = ((GridColor *)&rec->word[i])->word;
    i++;
    prim->r3 = ((GridColor *)&rec->word[i])->half.rf;
    prim->g3 = ((GridColor *)&rec->word[i])->word >> 8;
    prim->b3 = ((GridColor *)&rec->word[i])->word;
    i++;
    GpuAppendPrim(D_8014933E, 0x24);
  }
  if (GetGraphType() == 1 || GetGraphType() == 2) {
    page = 0x225;
  } else {
    page = 0x95;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 0, page, 0);
  GpuAppendPrim(D_8014933E, 0xC);
}
