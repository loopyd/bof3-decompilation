#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work. The original reads the cell once for the eight corner
 * expressions, which the compiler merges into the single lui 0x1F80 plus lw 0x44
 * pair that lands in $v1, and three more times after the primitive stores, each
 * as its own pair, because the packed writes to the polygon may alias the cell.
 * The pointer therefore stays unqualified and every corner and uv byte
 * dereferences it in place. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared graphic-type query that selects the draw-mode texture page argument.
 * The original calls it once unconditionally and a second time only when the
 * first answer is not 1. */
s32 GetGraphType(void);

/* @source 0x801EFA84
 * @behavior Textured-quad draw of the MAGIC069 work object, reached by the jal
 *   at 0x801EF9CC inside func_801EF9C4, which then raises work byte 0xA by 4
 *   once per call until it stands at 0x18 and advances work byte 2 at 0x14. It
 *   takes no arguments, reads no incoming register and returns nothing.
 *   It reads the shared graphic type twice and, when either answer is 1 or 2,
 *   selects the draw-mode texture page 0x25, else 0x15; the two values are the
 *   family formula ((0 & 3) << 7) | 0x25 and ((0 & 3) << 5) | 0x15 with the
 *   selector fixed at 0. It links a 0xC-byte draw-mode packet with SetDrawMode
 *   at g_PrimCursor with dfe 0, dtd 1 and a null texture window, then queues it
 *   with GpuAppendPrim(2, 0xC).
 *   It then takes a fresh g_PrimCursor POLY_FT4 and seeds it with SetPolyFT4
 *   without marking it semi-transparent, and writes four corners around the work
 *   object screen halfwords 0x2E (x) and 0x30 (y): corners 0 and 2 keep the
 *   centre, corners 1 and 3 add 0x30 to x, and corners 2 and 3 add the work byte
 *   0xA to y, so the quad is 0x30 pixels wide and work byte 0xA pixels tall and
 *   grows downward while the caller advances that byte.
 *   The texture is tpage GetTPage(0, 0, 0x380, 0x100) and clut GetClut(0x20,
 *   0x1FA), and the uv rectangle is the 0x40-wide strip whose top is v 0x20 and
 *   whose bottom is work byte 0xA plus 0x20: u 0 on the corners 0 and 2, u 0x40
 *   on the corners 1 and 3, v 0x20 on the corners 0 and 1 and v 0xA + 0x20 on the
 *   corners 2 and 3. Every colour byte is 0x80, so the strip is drawn with an
 *   unmodulated grey tint, and the body finally queues the 0x28-byte polygon with
 *   GpuAppendPrim(2, 0x28) and returns, having stored nothing but the primitive.
 *   The 0x20-byte frame keeps $ra at 0x1C and $s0 at 0x18, with $s0 holding the
 *   quad pointer across the two texture-page lookups. The roles of the work
 *   halfwords 0x2E and 0x30 beyond this quad's screen origin, of work byte 0xA
 *   beyond the quad height and strip v-range, and of the texture page and clut
 *   beyond their use here remain unestablished, so the name describes the
 *   measured drawing effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawGreyTexturedQuadAtWorkHalfwords2EAnd30Sized30ByWorkByteABmagicMagic06903_801EFA84(
    void) {
  POLY_FT4 *primitive;
  s32 graph;

  if (GetGraphType() == 1 || GetGraphType() == 2) {
    graph = 0x25;
  } else {
    graph = 0x15;
  }
  SetDrawMode((DR_MODE *)g_PrimCursor, 0, 1, graph, 0);
  GpuAppendPrim(2, 0xC);

  primitive = (POLY_FT4 *)g_PrimCursor;
  SetPolyFT4(primitive);

  primitive->x0 = *(u16 *)(g_battle_work + 0x2E);
  primitive->y0 = *(u16 *)(g_battle_work + 0x30);
  primitive->x1 = *(u16 *)(g_battle_work + 0x2E) + 0x30;
  primitive->y1 = *(u16 *)(g_battle_work + 0x30);
  primitive->x2 = *(u16 *)(g_battle_work + 0x2E);
  primitive->y2 = *(u16 *)(g_battle_work + 0x30) + g_battle_work[0xA];
  primitive->x3 = *(u16 *)(g_battle_work + 0x2E) + 0x30;
  primitive->y3 = *(u16 *)(g_battle_work + 0x30) + g_battle_work[0xA];

  primitive->tpage = GetTPage(0, 0, 0x380, 0x100);
  primitive->clut = GetClut(0x20, 0x1FA);

  primitive->u0 = 0;
  primitive->v0 = 0x20;
  primitive->u1 = 0x40;
  primitive->v1 = 0x20;
  primitive->u2 = 0;
  primitive->v2 = g_battle_work[0xA] + 0x20;
  primitive->u3 = 0x40;
  primitive->v3 = g_battle_work[0xA] + 0x20;

  primitive->r0 = 0x80;
  primitive->g0 = 0x80;
  primitive->b0 = 0x80;

  GpuAppendPrim(2, 0x28);
}
