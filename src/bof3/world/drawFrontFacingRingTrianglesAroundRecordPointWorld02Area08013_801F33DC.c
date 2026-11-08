#include "bof3/bof3.h"
#include "gpu/prim.h"

/* Shared two-point cross-product test at 0x801AFF90: it reads three
 * two-halfword points and returns the low halfword of
 * (b - a) x (c - b), which is the side the third point falls on. The low
 * halfword is sign-extended before the return, so the caller compares it as a
 * signed 16-bit quantity.
 * @source 0x801AFF90 */
extern s16 func_801AFF90(const void* a, const void* b, const void* c);

/* Shared libgpu POLY_F3 setter at 0x8017A954: it writes the four-word packet
 * header and the flat-triangle primitive code that this body then fills.
 * @source 0x8017A954 */
extern void func_8017A954(void* prim);

/* @source 0x801F33DC
 * @behavior Opens the drawing environment of the record render step: the
 *           texture page GetTPage(0, 1, 0x3C0, 0) is published through
 *           SetDrawMode on the shared primitive cursor with dither enabled and
 *           the twelve-byte primitive that step queues is then appended. It
 *           then runs sixteen iterations over the closed ring of sixteen
 *           four-byte points stored at +0x2C of the passed work record: each
 *           iteration passes the record point at +0x24 and the ring points at
 *           index i and at index (i + 1) masked with 0xF to the shared
 *           cross-product test at 0x801AFF90 and draws one primitive only
 *           while that test reads positive, so the back-facing triangles of
 *           the fan are dropped. The drawn primitive is a semi-transparent
 *           flat triangle taken from the shared primitive cursor and set
 *           through the shared POLY_F3 setter at 0x8017A954: its first vertex
 *           is the record point at +0x24, its second and third are the two
 *           ring points at +0x2C and +0x2E of index i and of index (i + 1)
 *           masked with 0xF, and its three colour bytes are all the record
 *           byte at +0x22. The triangle is then appended as a twenty-byte
 *           primitive. Takes the work record base as its only argument and
 *           returns nothing; its 0x30-byte frame holds $ra at 0x2C and the
 *           five saved registers across the calls.
 *
 * Name evidence: the single caller is the exact render step
 * seedPublishedRecordWordsFromSavedTripletAndRearmCountdownThenIncrementWorkByte1World02Area08013_801F2F50,
 * which passes the work record base it publishes in D_801F44A0, so the
 * argument role and the record view come from a matched body. The fan, the
 * ring of sixteen points, the wrap at index (i + 1) masked with 0xF and the
 * front-facing filter come from this body own loads, cross-product call and
 * branch. The colour and the two triangle vertices come from the stores into
 * the POLY_F3 packet, which is the four-word setter the body calls first.
 *
 * Shape evidence: statement order, operand forms and the primitive fill follow
 * the exact sibling func_801F2E04_area02613 (world00/area026/13 at 0x801F2E04),
 * which opens the same drawing environment, takes the cursor into a POLY_F3,
 * sets it through the same 0x8017A954 setter, marks it semi-transparent and
 * fills the same vertex and colour fields from a two-halfword point view; the
 * record-relative offsets, the ring size 0x10, the wrap mask 0xF, the texture
 * page 0x3C0 and the appended sizes 0xC and 0x14 are this function own bytes.
 * The clone lever is declined on one payload proof: the leading 0x40 bytes of
 * these 0x150 bytes occur only at payload offset 0x7DC of the 396 payloads
 * under out/binaries, and the whole span occurs only there too, so no
 * byte-identical twin of the range exists anywhere in the tree.
 *
 * Ring counter evidence: the original truncates the ring index through an
 * explicit 8-bit cast in exactly two places, the index scaled by four and the
 * loop test, and it frames both as `andi` followed by the scaled shift and by
 * `sltiu`; the increment itself adds one to the untruncated register, which an
 * 8-bit counter cannot carry. A 32-bit counter with those two casts therefore
 * reproduces the loop head, the wrap and the unsigned test, and a u8-typed
 * counter measured 71/84 because it forces the mask in front of the
 * increment.
 * @status partial
 * @match 91.67
 * @residual 77/84 instructions, 336 -> 332 bytes. Two residual classes, both
 *           measured and stable across the swept shapes. First, two address
 *           builds from the scaled ring offset print their commutative `addu`
 *           operands base-second where the original prints the record base
 *           first; index-first, base-first, pointer-local, halfword-view,
 *           struct-view and integer-domain spellings of those two addresses
 *           all measured the same operand order. Second, the original keeps
 *           the ring counter in a caller-saved register and copies the
 *           incremented value into a callee-saved register in the
 *           cross-product call delay slot, while this build increments
 *           straight into the callee-saved register and so is one instruction
 *           shorter.
 */
void drawFrontFacingRingTrianglesAroundRecordPointWorld02Area08013_801F33DC(
    u8* record) {
  POLY_F3* prim;
  s32 i;

  SetDrawMode((DR_MODE*)g_PrimCursor, 0, 1, GetTPage(0, 1, 0x3C0, 0), 0);
  func_8014E5A0(1, 0xC);
  i = 0;
  do {
    if (func_801AFF90(record + 0x24, record + 0x2C + (u8)i * 4,
                      record + 0x2C + ((i + 1) & 0xF) * 4) > 0) {
      prim = (POLY_F3*)g_PrimCursor;
      func_8017A954(prim);
      SetSemiTrans(prim, 1);
      prim->x0 = *(u16*)(record + 0x24);
      prim->y0 = *(u16*)(record + 0x26);
      prim->x1 = *(u16*)(record + (u8)i * 4 + 0x2C);
      prim->y1 = *(u16*)(record + (u8)i * 4 + 0x2E);
      prim->x2 = *(u16*)(record + ((i + 1) & 0xF) * 4 + 0x2C);
      prim->y2 = *(u16*)(record + ((i + 1) & 0xF) * 4 + 0x2E);
      prim->r0 = prim->g0 = prim->b0 = record[0x22];
      func_8014E5A0(1, 0x14);
    }
    i++;
  } while ((u8)i < 0x10u);
}
