#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Resident helpers this overlay calls, declared the way its siblings declare
 * them: func_801AFF04 projects the four-word point handed as arg0 into the
 * screen halfword pair written at arg1, and func_801AFFD8 converts the pair
 * handed at arg1 for that same point into arg2. */
void func_801AFF04(const void* arg0, void* arg1);
void func_801AFFD8(const void* arg0, void* arg1, void* arg2);

/* One 8-byte screen entry of the projection scratch. The original keeps two of
 * them at +0x00 and +0x08 of the scratch and reads back only their x/y
 * halfwords; the trailing four bytes are unmodelled here. */
typedef struct ScenarioScena0800ScreenPair {
  u16 x;
  u16 y;
  u16 unk_04;
  u16 unk_06;
} ScenarioScena0800ScreenPair;

/* The four-word point the record carries at +0x04. The body copies it into its
 * scratch as one 16-byte block, re-reads its third word from that scratch,
 * scales it and projects the point. */
typedef struct ScenarioScena0800Point4 {
  s32 x;
  s32 y;
  s32 z;
  s32 pad;
} ScenarioScena0800Point4;

/* The address-taken scratch that occupies +0x10..+0x2f of the original 0x40-byte
 * frame: the projector writes the point's screen pair into the first entry, the
 * converted size pair lands in the second, and +0x10..+0x1f holds the copy of
 * the record point. Both entries are passed by address, which is what keeps this
 * struct in memory. */
typedef struct ScenarioScena0800QuadScratch {
  ScenarioScena0800ScreenPair screen[2];
  ScenarioScena0800Point4     point;
} ScenarioScena0800QuadScratch;

/* The record the caller hands over: byte +0x03 is the colour channel written as
 * the quad's red, the four words at +0x04..+0x13 are the point this body scales
 * and projects, and the word at +0x24 is the size source, taken as its upper
 * bits. Everything the body does not exercise stays offset-anchored. */
typedef struct ScenarioScena0800QuadRecord {
  u8  unk_00[3];
  u8  color_03;
  s32 point_04[4];
  u8  unk_14[0x10];
  u32 size_24;
} ScenarioScena0800QuadRecord;

/* @source 0x801F77B4
 * @behavior Emits one screen-space translucent textured quad centred on a
 * record's projected point, for the only caller in this payload, the
 * table-refresh helper at 0x801F7E14 that walks the 0x40-entry record table at
 * 0x800E4800. It takes the shared primitive cursor 0x8014598C, initialises one
 * POLY_FT4 with SetPolyFT4 and marks it translucent with SetSemiTrans(prim, 1),
 * writes the record word at +0x24 shifted right by 6 into the first halfword of
 * its local screen pair, copies the record's 16-byte point at +0x04 into the
 * scratch, re-reads that copy's third word and shifts it left by 8, and hands
 * the point and the pair to func_801AFFD8, which writes the converted size pair
 * into the scratch's second entry; the same point is then projected through
 * func_801AFF04 over the first entry. The quad's corners are rebuilt from the
 * projected pair and the converted size: both x values are the projected x plus
 * and minus that size and both y values the projected y plus and minus the same
 * size, so each corner pair repeats a single stored halfword and the size
 * halfword at +0x00 of the second entry serves both axes. The CLUT is
 * GetClut(0xA0, 0x1E3) and the texture page GetTPage(0, 1, 0x2C0, 0x100); the
 * texels are the 0x20 by 0x20 cell at (0xE0, 0x30), stored as v0/v1, u0/u1,
 * u2/v2 and u3/v3 pairs in the order the original emits them, and the colour
 * bytes are green and blue cleared then red from the record byte at +0x03. It
 * closes by queueing the primitive through func_8014E5A0 with OT index 1 and
 * size 0x28. Takes the record pointer as its only argument, returns nothing, and
 * keeps a 0x40-byte frame for $ra and $s0..$s2 over the 0x20-byte scratch at
 * 0x10..0x2F. The record's remaining fields and the two resident helpers' wider
 * roles stay unknown; the shape follows the exact siblings
 * drawWorkRecordSemiTransparentQuadWorld03Area12513_801F2FC4 and this overlay's
 * own emitWorkObjectTexturedQuad_801F8BCC, which project a record point through
 * the same two helpers and emit the same textured quad with the same CLUT,
 * texture page and texel cell.
 * @status partial
 * @match 92.47
 * @residual Live audit: 86/93 instructions, 372 -> 372 bytes, first=+0x010c. The
 *   final texel/colour/queue block materialises the queue size 0x28 into $a1
 *   before the texel constants, so the shared 0xE0 constant takes $v0 and its two
 *   stores are issued next to each other, where the original keeps 0xE0 in $a1
 *   and interleaves the 0x14 store; the 0xFF constant and the colour source
 *   follow that register choice. Every earlier block, including the frame, the
 *   two Set calls, the size store, the 16-byte point copy with its read-back
 *   shift, both projector calls, the corner block and the CLUT/texture-page
 *   calls, is instruction-identical. Measured texel shapes with this first=:
 *   explicit v0/v1,u0/u1,u2/v2,u3/v3 order (retained) 86; setUVWH macro 84;
 *   chained texel pairs 83; colour block before the texel block 84; volatile
 *   colour stores 86 with the queue constant left after the call; colour written
 *   red-first 86; u2 before u1 86. All keep the same size and first
 *   difference, so the residual is the scheduler's constant-register choice, not
 *   the statement shape.
 */
void emitCenteredTranslucentQuadFromProjectedPointAndScaledSizeScenarioScena0800_801F77B4(void* arg0) {
  ScenarioScena0800QuadScratch       scratch;
  const ScenarioScena0800QuadRecord* record;
  POLY_FT4*                          prim;

  record = (const ScenarioScena0800QuadRecord*)arg0;
  prim = (POLY_FT4*)g_PrimCursor;

  SetPolyFT4(prim);
  SetSemiTrans(prim, 1);

  scratch.screen[0].x = (u16)(record->size_24 >> 6);
  scratch.point = *(const ScenarioScena0800Point4*)((const u8*)record + 0x04);
  /* The original stores the copy and then re-reads this word from the scratch
   * before shifting it, so the read is volatile and the store is not. */
  scratch.point.z = *(volatile s32*)((u8*)&scratch.point + 8) << 8;
  func_801AFFD8(&scratch.point, &scratch.screen[0], &scratch.screen[1]);
  func_801AFF04(&scratch.point, &scratch.screen[0]);

  prim->x0 = prim->x2 = scratch.screen[0].x - scratch.screen[1].x;
  prim->x1 = prim->x3 = scratch.screen[0].x + scratch.screen[1].x;
  prim->y0 = prim->y1 = scratch.screen[0].y - scratch.screen[1].x;
  prim->y2 = prim->y3 = scratch.screen[0].y + scratch.screen[1].x;
  prim->clut = GetClut(0xa0, 0x1e3);
  prim->tpage = GetTPage(0, 1, 0x2c0, 0x100);
  prim->v0 = 0x30;
  prim->v1 = 0x30;
  prim->u0 = 0xe0;
  prim->u1 = 0xff;
  prim->u2 = 0xe0;
  prim->v2 = 0x4f;
  prim->u3 = 0xff;
  prim->v3 = 0x4f;
  prim->g0 = 0;
  prim->b0 = 0;
  prim->r0 = record->color_03;

  func_8014E5A0(1, 0x28);
}
