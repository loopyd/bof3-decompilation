#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Work-record pointer cell at 0x801F21F0, the last word of this payload. The
 * overlay frame loop at 0x801EEC28 publishes the selected 0x2C-byte bmagic work
 * record into it, and the draw branch of the dispatcher at 0x801EF768 re-reads
 * it before calling here, so this body binds the cell by name. The prelude takes
 * one pointer load after the rand call and reads every field of that record
 * through it; the loop re-materialises the cell for the primitive linker because
 * a call sits between the two groups. The cell sits inside the payload's
 * trailing `textbin` row labelled D_801F05A4, which no map row names, so the
 * reference binds through this target's generated undefined-symbol table. */
extern u8 *D_801F21F0; /* @source 0x801F21F0 @kind data */

/* Scratchpad radius cell at 0x1F800004. The body stores it through a pointer
 * local and then derives the loop's centre base from that same pointer by two
 * halfwords, which is why the original materialises this address in a register
 * (`lui` plus `addiu`) instead of the `lui $at` form every other scratchpad
 * store below uses. The published value is the work record unsigned byte 0x06
 * plus a rand result masked with 3, and the loop reads it back as a signed
 * halfword, so the two fans it scales stay relative to the record radius. */
extern u16 D_1F800004; /* @source 0x1F800004 @kind data */

/* Colour table at 0x801F0644: three-byte triples, four entries per row, indexed
 * by the work record unsigned byte 0x04 as the row and byte 0x03 as the entry,
 * which is the record byte 0x03 times three plus byte 0x04 times twelve the
 * original scales. Its three bytes are the red, green and blue the body
 * multiplies by the record unsigned byte 0x05 and publishes as the scratchpad
 * tint triplet. The declared entry shape is load-bearing: indexing the same
 * bytes through a flat `u8 []` view leaves the compiler a 16-byte unused local
 * slot that grows the frame to 0x40, while this form keeps the frame at the
 * original's 0x30 with the identical instruction stream. The table sits inside
 * the payload's trailing `textbin` row, so it binds through this target's
 * generated undefined-symbol table and needs no map row. */
extern u8 D_801F0644[][4][3]; /* @source 0x801F0644 @kind data */

/* Shared primitive seed helper: the exact siblings at 0x801EFBF8 and 0x801EFE2C
 * seed their gouraud POLY_G3 the same way through their own helpers at
 * 0x8017AA94 and 0x8017AAC8, and the fan at emi/bmagic/magic046/03 0x801EFC48
 * calls this very address for the same gouraud triangle. No in-image symbol
 * names 0x8017A97C, so it keeps its address name. */
extern void func_8017A97C(void *arg0); /* @source 0x8017A97C */

/* Shared grid-cell primitive linker: it resolves the (x, y) cell through the
 * shared cell-size pair, links the primitive at g_PrimCursor with the given kind
 * and advances the cursor by the given size, which is why the body re-reads
 * g_PrimCursor at the top of every iteration. This body links a 0xC-byte
 * draw-mode packet and then one 0x1C-byte packet per triangle. */
void func_80155A08(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Shared phase samplers: both return the sample of a 12-bit phase in the same
 * fixed-point scale as the `>> 12` products below. Each triangle takes the
 * 0x801782FC sample for its x vertices and the 0x801783C8 sample for its y
 * vertices, and the phase steps by 0x200 per triangle, so eight triangles close
 * the 0x1000 circle. */
s32 func_801782FC(s32 arg0);
s32 func_801783C8(s32 arg0);

/* PsyQ random generator at 0x8017E3D4, bound through this target's generated
 * undefined-funcs row and declared the way the other bmagic sources declare
 * it. Only the low two bits of its result reach the radius cell. */
extern int rand(void);

/* Shared graphic-type query used for the draw-mode tpage argument. */
s32 GetGraphType(void);

/* Naming and shape evidence: the prelude is the one the two exact siblings of
 * this target share, with their tint publication replaced by the colour-table
 * lookup and their argument-driven phase by a fixed count. The name states the
 * eight-triangle fan, the published centre, the record byte 0x06 plus random
 * radius and the table tint scaled by record byte 0x05. */
/* @source 0x801F00C4
 * @behavior Battle-magic drawing step. Its only in-image route is the draw
 *           branch of the dispatcher at 0x801EF768, which calls it with no
 *           arguments when the record byte 0 has bit 0 set and byte 2 is
 *           non-zero, so it reads no incoming register and returns nothing.
 *           It selects the draw-mode texture page 0xA5 when the shared graphic
 *           type is 1 or 2 and 0x35 otherwise, installs it at g_PrimCursor with
 *           dfe 0 and dtd 1, and links a 0xC-byte draw-mode packet at the
 *           record words 0x14 and 0x18 with kind 2.
 *           It then publishes the burst into the scratchpad: the radius cell
 *           0x1F800004 takes record byte 0x06 plus a rand result masked with 3,
 *           the centre cells 0x1F800000 and 0x1F800002 take record halfwords
 *           0x20 and 0x22, the tint triplet 0x1F800008/0x1F80000A/0x1F80000C
 *           takes the colour-table triple at index record byte 0x03 times three
 *           plus byte 0x04 times twelve multiplied by record byte 0x05, and the
 *           alpha cell 0x1F80000E takes record byte 0x05 alone.
 *           It emits eight triangles, one per 0x200 step of the 0x1000 circle,
 *           so the wedges close a full disc. Each triangle is a
 *           semi-transparent gouraud POLY_G3 seeded by func_8017A97C whose
 *           first vertex is the published centre and whose second and third are
 *           that centre plus the 0x801782FC and 0x801783C8 samples of the
 *           current and next phase, scaled by the signed radius cell and
 *           shifted right 12. The first vertex takes the published tint
 *           triplet, the other two take the published alpha byte in all three
 *           colour components, and every iteration links a 0x1C-byte packet at
 *           the record words 0x14 and 0x18 with kind 2.
 *           The body keeps a 0x30-byte frame for $ra and four saved registers,
 *           writes no state outside the scratchpad cells and the primitives it
 *           emits, and is not called by any other payload span.
 * @status partial
 * @match 87.56
 * @residual 176 of 201 instructions match and the build is 804 bytes, equal to
 *           the original's 804. The draw-mode arm, the whole scratchpad prelude
 *           and the eight-iteration body match instruction for instruction,
 *           and so does the 0x30-byte frame: the colour table carries a
 *           declared entry shape because a flat byte view of the same bytes
 *           left a 16-byte unused local slot that grew the frame to 0x40.
 *           What remains is one allocation class. The original keeps the loop
 *           counter in $s2 and the centre base in $s3, while this build swaps
 *           the pair, so every differing instruction is that same register
 *           name and no instruction, store, constant or call differs. Probes
 *           that changed only allocation shape left the swap standing: both
 *           declaration orders of the locals, the counter initialised at the
 *           top of the body and just above the loop, the counter typed u16, u8,
 *           s16 and u32, the phase local typed u32, the exit comparison written
 *           reversed, the centre base built from the literal scratchpad cell
 *           instead of the radius pointer, and the record pointer spelled
 *           inline. The u16, u8 and s16 counters do not swap the pair either;
 *           they add a fifth saved register and four instructions. Body shape
 *           is therefore closed and the residual is the register pair alone.
 */
void drawTranslucentFanOfEightGouraudTrianglesFromPublishedCenterAtRadiusWorkByte6PlusRandomTintFromTableOfWorkBytes3And4TimesWorkByte5BmagicMagic07303_801F00C4(
    void) {
  u8 *record;
  u8 *primitive;
  u16 *cell;
  u16 *center;
  s32 tpage;
  s32 phase;
  s32 i;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    tpage = 0xA5;
  } else {
    tpage = 0x35;
  }

  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, tpage, 0);
  func_80155A08(*(s32 *)(D_801F21F0 + 0x14), *(s32 *)(D_801F21F0 + 0x18), 2,
                0xC);

  i = 0;

  cell = &D_1F800004;
  *cell = (rand() & 3) + D_801F21F0[6];
  record = D_801F21F0;
  *(u16 *)0x1F800000 = *(u16 *)(record + 0x20);
  *(u16 *)0x1F800002 = *(u16 *)(record + 0x22);
  *(u16 *)0x1F800008 = D_801F0644[record[4]][record[3]][0] * record[5];
  *(u16 *)0x1F80000A = D_801F0644[record[4]][record[3]][1] * record[5];
  *(u16 *)0x1F80000C = D_801F0644[record[4]][record[3]][2] * record[5];
  *(u16 *)0x1F80000E = record[5];
  center = cell - 2;

  do {
    primitive = g_PrimCursor;
    func_8017A97C(primitive);
    SetSemiTrans(primitive, 1);

    *(u16 *)(primitive + 8) = center[0];
    *(u16 *)(primitive + 0xA) = center[1];
    phase = i << 9;
    *(u16 *)(primitive + 0x10) =
        center[0] + ((func_801782FC(phase) * *(s16 *)(center + 2)) >> 12);
    *(u16 *)(primitive + 0x12) =
        center[1] + ((func_801783C8(phase) * *(s16 *)(center + 2)) >> 12);
    i++;
    phase = i << 9;
    *(u16 *)(primitive + 0x18) =
        center[0] + ((func_801782FC(phase) * *(s16 *)(center + 2)) >> 12);
    *(u16 *)(primitive + 0x1A) =
        center[1] + ((func_801783C8(phase) * *(s16 *)(center + 2)) >> 12);

    primitive[4] = center[4];
    primitive[5] = center[5];
    primitive[6] = center[6];
    primitive[0xC] = center[7];
    primitive[0xD] = center[7];
    primitive[0xE] = center[7];
    primitive[0x14] = center[7];
    primitive[0x15] = center[7];
    primitive[0x16] = center[7];

    func_80155A08(*(s32 *)(D_801F21F0 + 0x14), *(s32 *)(D_801F21F0 + 0x18), 2,
                  0x1C);
  } while (i < 8);
}
