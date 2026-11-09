#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The original materialises the cell once, after the primitive
 * cursor load, keeps the pointer in $v1 and reaches both halfwords of this body
 * through it with zero-base displacements of 0x2E and 0x30. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Scratchpad screen-centre cache at 0x1F80000C and 0x1F80000E. This body
 * publishes the work object halfwords 0x2E and 0x30 into the pair and then
 * re-reads one halfword per corner expression while it builds the two
 * primitives. The original addresses each halfword absolutely, with its own
 * lui 0x1F80 plus the load or store at that halfword's offset, and never keeps a
 * base register. Two other spellings measured worse: a bare scratchpad constant
 * for the pair (37/92 instructions at 320 bytes) and one named object for the
 * pair (57/92 at 364 bytes), both because the compiler hoisted the addresses
 * into saved registers, with lui/ori for the constants and a materialised member
 * base for the object. The two cells are therefore plain scalar objects, the
 * form the sibling
 * drawTranslucentTexturedQuadAtWorkHalfwords2EAnd30ScaledByWorkByte9TintedByWork
 * ByteATimesBytes5d5e5fBmagicMagic04403_801F06D8 shows for the same scratchpad
 * cells (its own metadata recorded 100.00 percent when this note was written).
 * Neither needs a map row: an address-encoded D_ name resolves from its
 * own spelling. Both stay volatile, without which the compiler keeps the first
 * read of each cell in a register and drops the three later reloads (measured
 * 54/92 at 304 bytes), while the original reads each cell once per corner. */
extern volatile u16 D_1F80000C; /* @source 0x1F80000C @kind data */
extern volatile u16 D_1F80000E; /* @source 0x1F80000E @kind data */

/* Scratchpad shade halfword at 0x1F800006, the cell this body publishes 0x80 into
 * and re-reads for every colour byte. The original keeps a pointer to it in $s0
 * across both primitive calls and loads through it with a zero displacement, so
 * the address is taken once here rather than re-formed per access. */
extern u16 D_1F800006; /* @source 0x1F800006 @kind data */

/* Main-executable primitive initialiser at 0x8017AA80, the same helper the
 * main-executable lift drawFlatLinePrim calls before it writes the colour bytes
 * and the two corner points of one flat 0x10-byte primitive. It takes the packet
 * pointer and returns nothing. */
void func_8017AA80(void *primitive);

/* Matching note: the corner colours are written through the packet pointer
 * without the volatile qualifier, while the four corners keep it. A volatile
 * packet pointer for all seven stores measured 89/92 instructions at 380 bytes,
 * because the three colour-byte stores then stop reorg filling the two
 * GpuAppendPrim delay slots; the non-volatile byte stores recover both slots.
 * The cell reads must stay volatile for the reason recorded above. */
/* @source 0x801F0104
 * @behavior Draw routine of the MAGIC069 work object, reached by the jal at
 *   0x801EEFCC inside this overlay's per-frame function func_801EEED4 when work
 *   byte 0 has bit 0 set and work byte 2 is at least 3. It takes no arguments,
 *   reads no incoming register and returns nothing.
 *   The body publishes 0x80 into the scratchpad shade halfword 0x1F800006, so
 *   that cell is a live pointer kept in $s0 across both calls, and copies the
 *   work object halfwords 0x2E and 0x30 into the scratchpad centre pair
 *   0x1F80000C and 0x1F80000E. It then emits two semi-transparent flat 0x10-byte
 *   line primitives through g_PrimCursor, each initialised by func_8017AA80 and
 *   SetSemiTrans with 1 before its fields are written, and each queued with
 *   GpuAppendPrim(3, 0x10).
 *   The first line runs from (0x0C - 4, 0x0E) to (0x0C + 4, 0x0E) and the
 *   second from (0x0C, 0x0E + 4) to (0x0C, 0x0E - 4), both in the order the
 *   original stores the corners, so the pair draws one grey cross with four-pixel
 *   arms around the cached work screen pair. Every colour byte is a fresh read of
 *   the 0x80 shade cell, three per line, which the original emits as one
 *   0($s0) load before each byte store. The 0x20-byte frame keeps $ra at 0x18 and
 *   $s0 and $s1 at 0x10 and 0x14, with $s1 holding the reloaded primitive cursor
 *   of each line. The roles of the work halfwords 0x2E and 0x30 beyond their use
 *   as this cross centre, and of the two scratchpad cells beyond caching them,
 *   remain unestablished, so the name describes the measured drawing effect.
 * @status partial
 * @match 97.85
 * @residual 91/92 instructions at the original 368-byte image, 372 bytes emitted,
 *   so the only difference is one extra instruction. Its site is the argument copy
 *   for the first func_8017AA80 call: the original keeps `addu $a0,$s1,$zero` in
 *   that call's delay slot with the 0x1F80000E publish store before it, while this
 *   candidate schedules the copy one slot earlier, into the load delay of the
 *   0x1F80000E publish load, and leaves a nop in the call delay slot. Every other
 *   instruction, the frame, the save set and the register assignment match.
 *   Source shapes that keep the copy with the call were searched: one named object
 *   for the centre pair, bare scratchpad constants, `u8 *` and `u32` packet
 *   pointers with raw field offsets, a non-volatile packet pointer with volatile
 *   corner stores, prototypes taking `void *`, `u32`, `LINE_F2 *` and the exact
 *   volatile type, a local packet copy, and the two publishes split around the
 *   call; all of them measured the same 91/92 with the copy in that slot.
 */
void drawTranslucentGreyCrossOfTwoFlatLinesAtWorkHalfwords2EAnd30BmagicMagic06903_801F0104(
    void) {
  volatile LINE_F2 *primitive;
  u16 *shade;
  u8 *work;

  shade = &D_1F800006;
  primitive = (volatile LINE_F2 *)g_PrimCursor;
  work = g_battle_work;
  *shade = 0x80;
  D_1F80000C = FIELD_REF(u16, work, 0x2E);
  D_1F80000E = FIELD_REF(u16, work, 0x30);
  func_8017AA80((void *)primitive);
  SetSemiTrans(primitive, 1);
  primitive->x0 = D_1F80000C - 4;
  primitive->y0 = D_1F80000E;
  primitive->x1 = D_1F80000C + 4;
  primitive->y1 = D_1F80000E;
  FIELD_REF(u8, (LINE_F2 *)primitive, 4) = *shade;
  FIELD_REF(u8, (LINE_F2 *)primitive, 5) = *shade;
  FIELD_REF(u8, (LINE_F2 *)primitive, 6) = *shade;
  GpuAppendPrim(3, 0x10);

  primitive = (volatile LINE_F2 *)g_PrimCursor;
  func_8017AA80((void *)primitive);
  SetSemiTrans(primitive, 1);
  primitive->x0 = D_1F80000C;
  primitive->y0 = D_1F80000E + 4;
  primitive->x1 = D_1F80000C;
  primitive->y1 = D_1F80000E - 4;
  FIELD_REF(u8, (LINE_F2 *)primitive, 4) = *shade;
  FIELD_REF(u8, (LINE_F2 *)primitive, 5) = *shade;
  FIELD_REF(u8, (LINE_F2 *)primitive, 6) = *shade;
  GpuAppendPrim(3, 0x10);
}
