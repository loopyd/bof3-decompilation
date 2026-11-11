#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Graphics-type query of the shared main executable, called twice by this
 * body. This target's generated undefined-funcs map binds the address, so no
 * map row is added. */
extern s32 GetGraphType(void); /* @source 0x8017B2B4 */

/* POLY_F4 initialiser of the shared main executable: stamps the packet the
 * primitive cursor points at with its tag, code and length. The exact
 * neighbours 0x801F3594 and 0x801F52D8 of the same corpus call it the same
 * way. Bound by the generated undefined-funcs map. */
extern void func_8017A9A4(u32 primitive); /* @source 0x8017A9A4 */

/* Four-byte blue-level table of this overlay's data blob at 0x801F3F5C, read
 * with the work-record byte at offset 0x02 as index. Its entries 0, 1, 2, 1
 * scale left by three to the run 0, 8, 16, 8. Flipping this body to C drops
 * the generated undefined-syms row the asm referenced it through, so the
 * target map carries it. */
extern u8 D_801F3F88[]; /* @source 0x801F3F88 @kind table */

/* Shared per-frame counter of main RAM, read here masked to its low three
 * bits. Already bound by the generated undefined-syms map. */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */

/* Scratchpad pointer cell holding the overlay work record this body reads twice.
 * The target map already binds it, and the symbol spelling keeps each access a
 * load of the cell instead of a constant base held across the calls. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind bss */

/* @source 0x801F33D8
 * @behavior Full-screen overlay tint emitter. Selects the draw-mode texture
 *           page 0x2A5 for graphics types 1 or 2 and 0xB5 otherwise through
 *           two successive GetGraphType calls, seeds that draw mode at the
 *           shared primitive cursor 0x8014598C with dfe 0, dtd 0 and no
 *           texture window, and appends its 0xC bytes under ordering class 3.
 *           It then stamps the same cursor through the shared POLY_F4
 *           initialiser 0x8017A9A4, sets that packet's semi-transparency bit,
 *           writes the four screen corners (0,0), (0x140,0), (0,0xF0) and
 *           (0x140,0xF0) as the eight halfwords at offsets 0x08 to 0x16, fixes
 *           the red byte at 0 and the green byte at 0x30, and takes the blue
 *           byte from the four-byte table at 0x801F3F88 indexed by the work
 *           record byte at offset 0x02 of the scratchpad pointer cell
 *           0x1F800044 and scaled left by three, then appends that 0x18-byte
 *           packet under the same ordering class 3. On every eighth frame,
 *           where the shared frame counter 0x80143E6C reads zero in its low
 *           three bits, it re-reads that work pointer and rewrites the same
 *           byte as (byte + 1) & 3, the dispatch byte the overlay dispatcher
 *           at 0x801F2C5C indexes its handler run with. The payload names no
 *           caller for it: no jal and no table word of this overlay holds
 *           0x801F33D8, so the overlay loader is the only entry that reaches
 *           it. Takes no arguments and returns nothing; the 0x20-byte frame
 *           keeps $ra at 0x1C and $s0 at 0x18.
 * @status partial
 * @match 88.89
 * @residual GCC 2.7.2 block scheduling of independent constant materialisations.
 *           The body is the full 72 instructions and the full 0x120 bytes, and
 *           every statement, operand, constant and store offset is the original
 *           one. What still differs is which of two independent `addiu`
 *           definitions the scheduler emits first, and the register that follows
 *           it: the original reads addiu a0,320 then addiu v0,48 where this
 *           source reads the 0x30 green pair first, which moves the
 *           0x1F800044 pointer cell from v1 to a0, and the original sets the
 *           second append's ordering class addiu a0,3 before the 0x801F3F88
 *           lookup while this source sets li a1,0x18 there. Reordering the two
 *           colour stores, the corner stores, a work-byte variable, a pointer
 *           variable, a width variable and POLY_F4 field stores in place of the
 *           casts all measured the same 64/72, and reading the work record after
 *           the corners instead of before costs the 72nd instruction that the
 *           pointer load has to fill.
 */
void setDrawModeThenDrawSemiTransFullScreenQuadWithTableIndexedBlueSteppingWorkByte2EveryEighthFrameWorld04Area17113_801F33D8(
    void) {
  s32 page;
  u32 prim;
  u8 *work;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    page = 0x2A5;
  } else {
    page = 0xB5;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 0, page, 0);
  GpuAppendPrim(3, 0xC);
  prim = (u32)g_PrimCursor;
  func_8017A9A4(prim);
  SetSemiTrans((void *)prim, 1);
  *(u8 *)(prim + 4) = 0;
  *(u8 *)(prim + 5) = 0x30;
  work = D_1F800044;
  *(s16 *)(prim + 8) = 0;
  *(s16 *)(prim + 10) = 0;
  *(s16 *)(prim + 12) = 0x140;
  *(s16 *)(prim + 14) = 0;
  *(s16 *)(prim + 16) = 0;
  *(s16 *)(prim + 18) = 0xF0;
  *(s16 *)(prim + 20) = 0x140;
  *(s16 *)(prim + 22) = 0xF0;
  *(u8 *)(prim + 6) = D_801F3F88[work[2]] << 3;
  GpuAppendPrim(3, 0x18);
  if ((D_80143E6C & 7) == 0) {
    D_1F800044[2] = (D_1F800044[2] + 1) & 3;
  }
}
