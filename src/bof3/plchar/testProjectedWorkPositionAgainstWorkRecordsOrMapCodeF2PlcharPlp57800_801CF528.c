#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. This body
 * reads the cell once, in its first statement, and keeps the pointer in a
 * register across all four call sites, so the cell has no later access here.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area's
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route * 2, and doubles each offset before it adds it, so both
 * bases stay their own symbol. This target generated
 * undefined_syms_auto.txt binds both addresses.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Slot search of the shared EXE image, outside this payload: it answers the
 * signed slot of the first eligible work record that holds the supplied point at
 * the supplied range, or -1 when no record does. This body passes range 0 and
 * turns the answer into a flag, so the original sign-extends the returned byte
 * and tests it against -1. */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Map code probe of the shared EXE image, outside this payload: it answers the
 * one-byte code of the map cell at the supplied signed column and row. This body
 * asks for the projected cell and for the two cells one unit further on, and
 * compares each answer with the code 0xF2. The cell offsets are signed
 * sixteen-bit values, so the original converts each retry index to that width. */
u8 func_80166CB0(s16 col, s16 row);

/* @source 0x801CF528
 * @behavior Handler of the front-end work object read through the scratchpad
 *           pointer cell 0x1F800044, reached only from func_801CEC78 in this
 *           payload: it projects the work position by adding the doubled
 *           movement offsets of 0x80181B94 and 0x80181B98 selected by the work
 *           route byte 0x08 to the work words 0x34 and 0x38. It hands that pair
 *           to the slot search func_801BDE14 at range 0 and starts its answer at
 *           one when the search reports any slot rather than -1. It then probes
 *           the map code of the projected cell through func_80166CB0: the cell
 *           itself always, the cell one column further whenever the low half of
 *           the first projected word is nonzero, and the cell one row further
 *           whenever the low half of the second projected word is nonzero, each
 *           probe raising the answer to one when the code reads 0xF2. It returns
 *           that answer as an unsigned byte and takes no arguments. Two jal
 *           sites inside func_801CEC78 of this payload reach it while that body
 *           steps the work route: at 0x801CECA8 after storing the route byte
 *           moved back by one and masked with 7, and at 0x801CECD8 after storing
 *           the route byte moved forward by two and masked with 7, and each site
 *           masks the returned byte and leaves that block when the answer is
 *           nonzero. The 0x30-byte frame keeps $ra at 0x28 and the six saved
 *           registers across all four call sites, the scratchpad cell load
 *           precedes frame creation, and the range constant is materialised
 *           before the projection tables are read. No word of this payload holds
 *           this address, so no code-pointer run names the body. The 0x134-byte
 *           span measured byte-identical, relocations included, to the payload
 *           spans of the same body already matched in plp012 at 0x801CEFF8,
 *           plp27a at 0x801CF724 and plp247 at 0x801CF940 (md5
 *           d023ff42864229d2a81a039107828d35), so the statement order and
 *           operand spelling here are those of those bodies written against this
 *           payload's own span.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 testProjectedWorkPositionAgainstWorkRecordsOrMapCodeF2PlcharPlp57800_801CF528(
    void) {
  u8* work;
  u8 route;
  s32 x;
  s32 y;
  u8 hit;

  work = D_1F800044;
  route = work[0x08];
  x = *(s32*)(work + 0x34) + D_80181B94[route * 2] * 2;
  y = *(s32*)(work + 0x38) + D_80181B98[route * 2] * 2;
  hit = (func_801BDE14(x, y, 0) != -1);
  if ((func_80166CB0(x >> 16, y >> 16) & 0xFF) == 0xF2) {
    hit = 1;
  }
  if ((x & 0xFFFF) != 0) {
    if ((func_80166CB0((x >> 16) + 1, y >> 16) & 0xFF) == 0xF2) {
      hit = 1;
    }
  }
  if ((y & 0xFFFF) != 0) {
    if ((func_80166CB0(x >> 16, (y >> 16) + 1) & 0xFF) == 0xF2) {
      hit = 1;
    }
  }
  return hit;
}
