#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-record pointer cell this overlay publishes and every record
 * handler of the target reads. The original materialises the cell with a fresh
 * `lui`/`lw` pair at every use, so the body keeps one pointer local per use.
 * @source 0x1F800044 @kind bss
 */
extern u8* D_1F800044;

/* Per-pass row table of this overlay: the two signed scale bytes that size the
 * quad through the unsigned work halfword 0x30, followed by the packed texture
 * state words the closing quad takes at the work record row index 0x32. The
 * original reloads the byte for every pass and leaves the sign extension to its
 * own instruction pair, which only a volatile view keeps.
 * @source 0x801F35D8 @kind data
 */
extern volatile u8 D_801F35D8[];

/* The four scratchpad corner vectors at 0x1F800014 that RotTransPers4 projects
 * into the packet corner pairs, one cell per component halfword. The original
 * writes every column halfword through its own literal address.
 * @source 0x1F800014 @kind data */
extern s16 D_1F800014;
/* @source 0x1F800016 @kind data */
extern s16 D_1F800016;
/* @source 0x1F800018 @kind data */
extern s16 D_1F800018;
/* @source 0x1F80001C @kind data */
extern s16 D_1F80001C;
/* @source 0x1F80001E @kind data */
extern s16 D_1F80001E;
/* @source 0x1F800020 @kind data */
extern s16 D_1F800020;
/* @source 0x1F800024 @kind data */
extern s16 D_1F800024;
/* @source 0x1F800026 @kind data */
extern s16 D_1F800026;
/* @source 0x1F800028 @kind data */
extern s16 D_1F800028;
/* @source 0x1F80002C @kind data */
extern s16 D_1F80002C;
/* @source 0x1F80002E @kind data */
extern s16 D_1F80002E;
/* @source 0x1F800030 @kind data */
extern s16 D_1F800030;

/* Texture-state filler of the main executable: it writes the packed texture
 * page, texture window and colour lookup word into the packet and starts its
 * texture-state run with the given kind.
 * @source 0x80155560
 */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2);

/* Primitive linker of the main executable: it resolves the record position pair
 * through the shared cell-size pair, links the packet at the cursor with the
 * given kind and advances the cursor by the given size.
 * @source 0x80155A08
 */
extern void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F313C
 * @behavior Two-pass textured quad drawer of the byte-2 state handler
 *           0x801F2F34 of this overlay, which runs it once per frame with mode
 *           0 and mode -1.
 *           It first fills the flat coordinate of the four scratchpad vectors at
 *           0x1F800014 from the work record published at the pointer cell
 *           0x1F800044: work byte 0x08 set folds the work halfword 0x3A as
 *           ((halfword - 0x80) << 7) | 0x40 into the y column, and byte 0x08
 *           clear folds the work halfword 0x36 the same way into the x column.
 *           The depth column then takes the unsigned work halfword 0x3E less
 *           0x180 for the two leading vectors and the same halfword unchanged
 *           for the two trailing ones.
 *           Each of the two passes tags one POLY_FT4 at the shared primitive
 *           cursor with flat shading and fills the free column with the work
 *           halfword 0x36 shifted by the pass index when work byte 0x08 is set,
 *           or the work halfword 0x3A when it is clear, offset by the product of
 *           the unsigned work halfword 0x30 with the signed table byte
 *           0x801F35D8[pass] plus 0x40 for the two leading vectors and minus
 *           0x40 for the two trailing ones. It then projects the four vectors
 *           through RotTransPers4 into the packet corner pairs, fills the
 *           texture state through func_80155560 from 0x14500000 | (work byte
 *           0x08 << 21) | (work halfword 0x2E + pass), and links the quad
 *           through func_80155A08 at the work words 0x34 and 0x38 with the
 *           caller mode and size 0x28.
 *           When the signed work halfword 0x32 is nonzero it emits one further
 *           POLY_FT4 of the same orientation pair, folded as
 *           (halfword << 7) - 0x40C0 and ((halfword - 0x81) << 7) | 0x40, with
 *           both depths taken from the unsigned work halfword 0x3E less 0x140,
 *           the texture state word 0x801F35D8[0x32] and the linked kind 0.
 *           Takes the render mode by value and returns nothing; its 0x50-byte
 *           frame keeps $ra at 0x48 and $s0 through $s5 below it, and the depth
 *           and flag results of the projections live at 0x28 and 0x2C.
 * @status partial
 * @match 69.93
 * @residual 193/276 instructions match at 1104->1100 bytes. The entry work-record
 *           load now agrees. The first mismatch is the earlier save and copy of
 *           the mode argument. The depth anchor still materialises before the
 *           orientation branch instead of in the shared block, and the compiler
 *           hoists the pass-index initialization out of the two arms. The row
 *           pointer and x-column base occupy $s2 and $s1 here instead of $s1 and
 *           $s2. The pass-loop record pointer and arithmetic temporaries also
 *           differ in allocation and scheduling. No byte match is established.
 */
void drawTwoPassTexturedQuadAtWorkHalfwords36And3aWithHalfword30SpreadAndRow32QuadWorld03Area14713_801F313C(
    s32 arg0) {
  s16* vx;
  s16* vy;
  s16* vz;
  u8* rec0;
  u8* rec1;
  u8* rec2;
  u8* rec3;
  u8* rec4;
  u8* rec5;
  u8* rec6;
  u8* rec7;
  u8* rec8;
  u8* depthRec;
  POLY_FT4* prim;
  s32 i;
  s32 value;
  s32 prod;
  s32 off;
  volatile u8* row;
  s32 scale;
  long depth;
  long flag;

  vz = (s16 *)&D_1F800018;
  rec0 = D_1F800044;
  if (rec0[8] != 0) {
    value = ((*(s16*)(rec0 + 0x3A) - 0x80) << 7) | 0x40;
    D_1F80002E = value;
    D_1F800026 = value;
    D_1F80001E = value;
    D_1F800016 = value;
    i = 0;
  } else {
    value = ((*(s16*)(rec0 + 0x36) - 0x80) << 7) | 0x40;
    D_1F80002C = value;
    D_1F800024 = value;
    D_1F80001C = value;
    D_1F800014 = value;
    i = 0;
  }
  depthRec = D_1F800044;
  value = *(u16*)(depthRec + 0x3E) - 0x180;
  D_1F800020 = value;
  *vz = value;
  value = *(u16*)(depthRec + 0x3E);
  D_1F800030 = value;
  D_1F800028 = value;
  vx = (s16 *)((u8 *)vz - 4);
  vy = (s16 *)((u8 *)vz - 2);
  row = D_801F35D8;
  do {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);
    rec1 = D_1F800044;
    if (rec1[8] != 0) {
      off = (i - 0x80);
      scale = *row;
      prod = *(u16*)(rec1 + 0x30) * (s8)scale + 0x40;
      value = ((*(s16*)(rec1 + 0x36) + off) << 7) - prod;
      vx[8] = value;
      vx[0] = value;
      scale = *row;
      prod = *(u16*)(rec1 + 0x30) * (s8)scale - 0x40;
      value = ((*(s16*)(rec1 + 0x36) + off) << 7) - prod;
      vx[12] = value;
      vx[4] = value;
    } else {
      off = (i + 0x7F);
      scale = *row;
      prod = *(u16*)(rec1 + 0x30) * (s8)scale + 0x40;
      value = ((*(s16*)(rec1 + 0x3A) - off) << 7) + prod;
      vy[8] = value;
      vy[0] = value;
      scale = *row;
      prod = *(u16*)(rec1 + 0x30) * (s8)scale - 0x40;
      value = ((*(s16*)(rec1 + 0x3A) - off) << 7) + prod;
      vy[12] = value;
      vy[4] = value;
    }
    RotTransPers4((SVECTOR *)&vx[0], (SVECTOR *)&vx[4],
                  (SVECTOR *)&vx[8], (SVECTOR *)&vx[0xC],
                  (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                  (long*)&prim->x3, &depth, &flag);
    rec2 = D_1F800044;
    func_80155560((*(s16*)(rec2 + 0x2E) + i) | 0x14500000 | (rec2[8] << 21),
                  prim, 1);
    rec3 = D_1F800044;
    func_80155A08(*(s32*)(rec3 + 0x34), *(s32*)(rec3 + 0x38), (s8)arg0, 0x28);
    row++;
    i++;
  } while (i < 2);
  rec4 = D_1F800044;
  if (*(s16*)(rec4 + 0x32) != 0) {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);
    rec5 = D_1F800044;
    if (rec5[8] != 0) {
      value = (*(s16*)(rec5 + 0x36) << 7) - 0x40C0;
      vx[8] = value;
      vx[0] = value;
      value = ((*(s16*)(rec5 + 0x36) - 0x81) << 7) | 0x40;
      vx[0xC] = value;
      vx[4] = value;
    } else {
      value = ((*(s16*)(rec5 + 0x3A) - 0x81) << 7) | 0x40;
      vy[8] = value;
      vy[0] = value;
      value = (*(s16*)(rec5 + 0x3A) << 7) - 0x40C0;
      vy[0xC] = value;
      vy[4] = value;
    }
    rec6 = D_1F800044;
    value = *(u16*)(rec6 + 0x3E) - 0x140;
    *(s16 *)0x1F800020 = value;
    *(s16 *)0x1F800018 = value;
    RotTransPers4((SVECTOR *)&vx[0], (SVECTOR *)&vx[4],
                  (SVECTOR *)&vx[8], (SVECTOR *)&vx[0xC],
                  (long*)&prim->x0, (long*)&prim->x1, (long*)&prim->x2,
                  (long*)&prim->x3, &depth, &flag);
    rec7 = D_1F800044;
    func_80155560(((u32*)D_801F35D8)[*(s16*)(rec7 + 0x32)], prim, 1);
    rec8 = D_1F800044;
    func_80155A08(*(s32*)(rec8 + 0x34), *(s32*)(rec8 + 0x38), 0, 0x28);
  }
}
