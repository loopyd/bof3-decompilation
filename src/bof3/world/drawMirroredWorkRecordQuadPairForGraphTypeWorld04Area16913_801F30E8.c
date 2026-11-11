#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Per-pass mirror scale bytes of this overlay's data blob, the two signed
 * bytes at 0x801F3AAC holding +1 and -1: the draw loop multiplies the work
 * record's halfword 0x30 by one of them per pass, so the two quads it emits
 * are mirrored about the record's x column. Interior words of the pointer
 * run declared whole at 0x801F3A90. */
/* @source 0x801F3AAC @kind data */
extern u8 D_801F3AAC[];

/* The scratchpad vector deck the four corners are built in and handed to
 * RotTransPers4, at the four 8-byte vectors starting at 0x1F800014. */
/* @source 0x1F800014 @kind unknown */
extern SVECTOR D_1F800014[];

/* Graph-type selector of the main executable; the draw mode depth of this
 * overlay's two-pass quad is 0x225 for graph types 1 and 2 and 0x95
 * otherwise. */
/* @source 0x8017B2B4 */
extern s32 GetGraphType(void);

/* Texture-state filler of the main executable: it completes the packet at the
 * given pointer with the packed texture page/colour word in its first
 * argument. */
/* @source 0x80155560 */
extern void func_80155560(u32 arg0, void* arg1, s32 arg2);

/* @source 0x801F30E8
 * @behavior Draw routine of this overlay's step handlers: it builds the
 *           scratchpad vector deck at 0x1F800014 from the work record
 *           published at the pointer cell 0x1F800044 and emits, in a
 *           two-pass loop, one 0x28-byte textured quad per pass after a
 *           0x0C-byte draw mode, both appended with kind 6.
 *           The record's route byte at +8 selects which axis the two deck
 *           columns share: when it is non-zero all four deck y values take
 *           ((halfword 0x3A - 0x80) << 7) | 0x40, and when it is zero all
 *           four deck x values take ((halfword 0x36 - 0x80) << 7) | 0x40.
 *           The deck depths then take the record's unsigned halfword 0x3E,
 *           corners 0 and 1 that halfword minus 0x180 and corners 2 and 3
 *           the halfword itself.
 *           Per pass the remaining axis is written as a 0x100-wide pair of
 *           columns around the record's halfword 0x36: corners 0 and 2 take
 *           ((halfword 0x36 - 0x7F) << 7) plus the mirrored offset and 0x40,
 *           corners 1 and 3 take ((halfword 0x36 - 0x80) << 7) plus that
 *           offset and -0x40. The offset is the record's unsigned halfword
 *           0x30 scaled by the pass's mirror byte from 0x801F3AAC, so the two
 *           quads slide to opposite sides of the record's column.
 *           RotTransPers4 projects the four deck vectors into the primitive's
 *           four vertex pairs, func_80155560 fills its texture state from
 *           0x12510000, the record's route byte shifted up 21 and 0x10C minus
 *           the pass index, and the draw mode is armed on the shared cursor
 *           before the loop: dfe 0, dtd 0, the depth chosen by GetGraphType
 *           (0x225 for graph types 1 and 2, 0x95 otherwise) and a fifth
 *           argument of zero on the stack. The record cell is re-read for
 *           every use, so the four loads are separate reads of 0x1F800044.
 *           Takes no arguments and returns nothing; the 0x48-byte frame keeps
 *           $ra at 0x44 and $s0 through $s4 below it.
 * @status partial
 * @match 87.44
 * @residual 174 of 199 original instructions match at 796 bytes. The first
 *           difference is the mirror-byte conversion at +0x188: this build
 *           emits xori/addiu for the signed-byte bias, while the original
 *           emits sll/sra. Both preserve the original unsigned byte load.
 *           The multiply low word and its addend use different registers and
 *           operand order, and the tail texture word is
 *           or-ed in the other operand order; none of the three changes a
 *           count.
 */
void drawMirroredWorkRecordQuadPairForGraphTypeWorld04Area16913_801F30E8(void) {
  u8* setupWork;
  u8* depthWork;
  u8* work;
  SVECTOR* v;
  s16* vy;
  POLY_FT4* prim;
  s32 i;
  s32 tpage;
  s32 offset;
  s32 value;
  s32 scale;
  long depth;
  long flag;

  /* Reuse the consumed cell address for its loaded record pointer, which is
   * what keeps the entry load above the frame setup. */
  setupWork = SPAD_ADDR(u8, 0x44u);
  setupWork = *(u8**)setupWork;
  if (setupWork[8] != 0) {
    value = ((FIELD_REF(s16, setupWork, 0x3A) - 0x80) << 7) | 0x40;
    SPAD_REF(s16, 0x16u) = SPAD_REF(s16, 0x1Eu) = SPAD_REF(s16, 0x26u) =
        SPAD_REF(s16, 0x2Eu) = value;
  } else {
    value = ((FIELD_REF(s16, setupWork, 0x36) - 0x80) << 7) | 0x40;
    SPAD_REF(s16, 0x14u) = SPAD_REF(s16, 0x1Cu) = SPAD_REF(s16, 0x24u) =
        SPAD_REF(s16, 0x2Cu) = value;
  }

  depthWork = SPAD_ADDR(u8, 0x44u);
  depthWork = *(u8**)depthWork;
  SPAD_REF(s16, 0x18u) = SPAD_REF(s16, 0x20u) =
      FIELD_REF(u16, depthWork, 0x3E) - 0x180;
  SPAD_REF(s16, 0x28u) = SPAD_REF(s16, 0x30u) = FIELD_REF(u16, depthWork, 0x3E);

  if (GetGraphType() == 1) {
    tpage = 0x225;
  } else if (GetGraphType() == 2) {
    tpage = 0x225;
  } else {
    tpage = 0x95;
  }
  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 0, tpage, 0);
  GpuAppendPrim(6, 0xC);
  i = 0;
  v = D_1F800014;
  vy = (s16*)v + 1;

  for (; i < 2; i++) {
    prim = (POLY_FT4*)g_PrimCursor;
    SetPolyFT4(prim);
    SetShadeTex(prim, 0);

    work = SPAD_ADDR(u8, 0x44u);
    work = *(u8**)work;
    if (work[8] != 0) {
      scale = D_801F3AAC[i];
      offset = FIELD_REF(u16, work, 0x30) * ((scale ^ 0x80) - 0x80);
      offset += 0x40;
      value = offset + ((FIELD_REF(s16, work, 0x36) - 0x7F) << 7);
      v[0].vx = v[2].vx = value;
      scale = D_801F3AAC[i];
      offset = FIELD_REF(u16, work, 0x30) * ((scale ^ 0x80) - 0x80);
      offset -= 0x40;
      value = offset + ((FIELD_REF(s16, work, 0x36) - 0x80) << 7);
      v[1].vx = v[3].vx = value;
    } else {
      scale = D_801F3AAC[i];
      offset = FIELD_REF(u16, work, 0x30) * ((scale ^ 0x80) - 0x80);
      offset += 0x40;
      value = offset + ((FIELD_REF(s16, work, 0x3A) - 0x7F) << 7);
      vy[0] = vy[8] = value;
      scale = D_801F3AAC[i];
      offset = FIELD_REF(u16, work, 0x30) * ((scale ^ 0x80) - 0x80);
      offset -= 0x40;
      value = offset + ((FIELD_REF(s16, work, 0x3A) - 0x80) << 7);
      vy[4] = vy[0xC] = value;
    }

    RotTransPers4(v, v + 1, v + 2, v + 3, (long*)&prim->x0, (long*)&prim->x1,
                  (long*)&prim->x2, (long*)&prim->x3, &depth, &flag);

    work = SPAD_ADDR(u8, 0x44u);
    work = *(u8**)work;
    func_80155560(0x12510000 | (0x10C - i) | (work[8] << 21), (void*)prim, 1);
    GpuAppendPrim(6, 0x28);
  }
}
