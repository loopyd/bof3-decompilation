#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. This body
 * reads the cell once, in its first statement, and keeps the pointer in a
 * register across all four shared calls, so the cell has no later access here.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area's
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route * 2, and doubles each offset before it adds it, so both
 * bases stay their own symbol.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Slot search of the shared EXE image, outside this payload: it answers the
 * signed slot of the first eligible work record that holds the supplied point at
 * the supplied range, or -1 when no record does. This body passes range 0 and
 * turns the answer into a flag, so the original sign-extends the returned byte
 * and tests it against -1. The lift at 0x801CE7B8 of this payload declares the
 * same prototype.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Test of the shared EXE image, outside this payload, that takes no arguments
 * and answers a signed byte the body tests against -1 the same way the slot
 * search is tested. No lifted body in this repository names it, so the role
 * stays address-anchored; the original sets up no argument register for this
 * call, which is why the prototype takes none. The lift at 0x801CE7B8 of this
 * payload declares the same prototype.
 */
s8 func_801B5AE4(void);

/* Map code probe of the shared EXE image, outside this payload: it answers the
 * one-byte code of the map cell at the supplied signed column and row. This body
 * asks for the projected cell and for the two cells one unit further on, and
 * compares each answer with the code run 0xF0, 0xF1, 0xF4, 0xF6, 0xF7. The cell
 * offsets are signed sixteen-bit values, so the original converts each retry
 * index to that width. No other lifted body of this payload declares this
 * prototype, so the argument widths here are this body's own evidence: the two
 * signed halves of the projected position, each retry widened by one before the
 * call re-narrows it.
 */
u8 func_80166CB0(s16 col, s16 row);

/* @source 0x801CF184
 * @behavior Step test of the front-end work object read through the scratchpad
 *           pointer cell 0x1F800044, run by this payload's step handler
 *           0x801CE698 from the two plain `jal` sites 0x801CE6C0 and 0x801CE6F0,
 *           each time with work byte 8 moved one step and then again with it
 *           moved two steps the other way. No code-pointer word of this payload
 *           holds this address. It projects the work position by adding the
 *           doubled movement offsets of 0x80181B94 and 0x80181B98 selected by
 *           the work route byte 0x08 to the work words 0x34 and 0x38, and hands
 *           that pair to the slot search func_801BDE14 at range 0, so the answer
 *           starts at one when the search reports any slot rather than -1. The
 *           no-argument test func_801B5AE4 raises that answer to one again when
 *           it reports anything other than -1. The body then probes the map code
 *           of the projected cell through func_80166CB0 and raises the answer to
 *           one when a code reads 0xF0, 0xF1, 0xF4, 0xF6 or 0xF7: the projected
 *           cell always, the cell one column further whenever the low half of the
 *           first projected word is nonzero, and the cell one row further
 *           whenever the low half of the second projected word is nonzero. It
 *           returns that answer as an unsigned byte and takes no arguments, and
 *           the 0x20-byte frame keeps $ra at 0x1C and $s2, $s1 and $s0 at 0x18,
 *           0x14 and 0x10 across the four calls, with the range constant 0
 *           materialised before the projection tables are read. The roles of the
 *           map codes, of the work-record slot search and of the helper test are
 *           not established, so those stay address-anchored. The payload
 *           plp034/00 holds the same 0x1A4-byte body at 0x801CFEAC and plp016/00
 *           at 0x801D02A0, all 105 words equal, and the plp034/00 exact lift
 *           seeded this source, so those twins are shape evidence and not a
 *           clone source.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 testProjectedWorkPositionAgainstWorkRecordsHelperOrMapCodesF0F1F4F6F7PlcharPlp34900_801CF184(
    void) {
  u8* work;
  u8 route;
  s32 x;
  s32 y;
  u8 code;
  u8 hit;

  work = D_1F800044;
  route = work[0x08];
  x = *(s32*)(work + 0x34) + D_80181B94[route * 2] * 2;
  y = *(s32*)(work + 0x38) + D_80181B98[route * 2] * 2;
  hit = (func_801BDE14(x, y, 0) != -1);
  if (func_801B5AE4() != -1) {
    hit = 1;
  }
  code = func_80166CB0(x >> 16, y >> 16);
  if (code == 0xF0 || code == 0xF1 || code == 0xF4 || code == 0xF6 ||
      code == 0xF7) {
    hit = 1;
  }
  if ((x & 0xFFFF) != 0) {
    code = func_80166CB0((x >> 16) + 1, y >> 16);
    if (code == 0xF0 || code == 0xF1 || code == 0xF4 || code == 0xF6 ||
        code == 0xF7) {
      hit = 1;
    }
  }
  if ((y & 0xFFFF) != 0) {
    code = func_80166CB0(x >> 16, (y >> 16) + 1);
    if (code == 0xF0 || code == 0xF1 || code == 0xF4 || code == 0xF6 ||
        code == 0xF7) {
      hit = 1;
    }
  }
  return hit;
}
