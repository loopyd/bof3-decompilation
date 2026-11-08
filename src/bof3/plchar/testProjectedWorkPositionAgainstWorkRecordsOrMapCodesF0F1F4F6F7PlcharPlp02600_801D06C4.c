#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. This body
 * reads the cell once, in its first statement, and keeps the pointer in a
 * register across all three shared calls, so the cell has no later access here.
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
 * the supplied range, or -1 when no record does. Its rows are the twenty
 * 0x74-byte front-end records at 0x80143FC8 whose byte 5 reads 0x30. This body
 * passes range 0 and turns the answer into a flag, so the original sign-extends
 * the returned byte and tests it against -1.
 */
s8 func_801BDE14(s32 x, s32 y, s32 range);

/* Second slot search of the shared EXE image, outside this payload: it takes no
 * arguments, projects the same active work area with the undoubled offsets of
 * 0x80181B94/0x80181B98 and walks the twenty 0x74-byte front-end records at
 * 0x80143FC8 whose byte 0 reads nonzero and whose byte 5 reads 0x17, passing each
 * one to the shared gate func_801BDD58 with the signed reference halfword at work
 * offset 0x3E and range 0. It answers the signed slot of the first record the
 * gate accepts, or -1 when none does. This body passes no arguments either and
 * folds the answer into the same flag against -1.
 */
s8 func_801B5AE4(void);

/* Map code probe of the shared EXE image, outside this payload: it answers the
 * one-byte code of the map cell at the supplied signed column and row. This body
 * asks for the projected cell and for the two cells one unit further on, and
 * classifies each answer. The cell offsets are signed sixteen-bit values, so the
 * original converts each retry index to that width.
 */
u8 func_80166CB0(s16 col, s16 row);

/* @source 0x801D06C4
 * @behavior Step test of the front-end work object read through the scratchpad
 *           pointer cell 0x1F800044, reached only from the work-byte-3 steps of
 *           this payload (0x801CFBD8 and 0x801D0484), which rewrite work byte
 *           0x08 and then read this answer to decide whether the rewritten route
 *           is walkable. It projects the work position by adding the doubled
 *           movement offsets of 0x80181B94 and 0x80181B98 selected by the work
 *           route byte 0x08 to the work words 0x34 and 0x38. It hands that pair
 *           to the slot search func_801BDE14 at range 0 and starts its answer at
 *           one when the search reports any slot rather than -1, and raises the
 *           same answer to one when the no-argument slot search func_801B5AE4
 *           reports any of its own slots rather than -1. It then probes the map
 *           code of the projected cell through func_80166CB0: the cell itself
 *           always, the cell one column further whenever the low half of the
 *           first projected word is nonzero, and the cell one row further
 *           whenever the low half of the second projected word is nonzero. Each
 *           probe raises the answer to one when the code reads 0xF0 or 0xF1
 *           (spelled (u8)(code + 0x10) below 2), 0xF4, or 0xF6 or 0xF7
 *           (spelled (u8)(code + 0xA) below 2), the five cell codes the payload
 *           treats as blocked. It returns that answer as an unsigned byte and
 *           takes no arguments, the range constant 0 for the first slot search
 *           being the only argument register it sets. The 0x20-byte frame keeps
 *           $ra at 0x1C and $s2..$s0 at 0x18..0x10 across the three shared
 *           calls. The 0x1A4-byte span is byte-identical to the copy at
 *           0x801D02A0 of plp016, 0x801CFEAC of plp034, 0x801D04C8 of plp056,
 *           0x801CF99C of plp267, 0x801CF184 of plp349, 0x801CF3C0 of plp467,
 *           0x801CF7A0 of plp567 and 0x801CF1A4 of plp678, every one of them
 *           still an unclaimed row, so those copies are evidence of a shared
 *           handler body rather than a lift of this target.
 * @status exact
 * @match 100.00
 * @residual none
 */
u8 testProjectedWorkPositionAgainstWorkRecordsOrMapCodesF0F1F4F6F7PlcharPlp02600_801D06C4(
    void) {
  u8* work;
  u8 route;
  u8 code;
  u8 hit;
  s32 x;
  s32 y;

  work = D_1F800044;
  route = work[0x08];
  x = *(s32*)(work + 0x34) + D_80181B94[route * 2] * 2;
  y = *(s32*)(work + 0x38) + D_80181B98[route * 2] * 2;
  hit = (func_801BDE14(x, y, 0) != -1);
  if (func_801B5AE4() != -1) {
    hit = 1;
  }
  code = func_80166CB0(x >> 16, y >> 16);
  if ((u8)(code + 0x10) < 2 || code == 0xF4 || (u8)(code + 0xA) < 2) {
    hit = 1;
  }
  if ((x & 0xFFFF) != 0) {
    code = func_80166CB0((x >> 16) + 1, y >> 16);
    if ((u8)(code + 0x10) < 2 || code == 0xF4 || (u8)(code + 0xA) < 2) {
      hit = 1;
    }
  }
  if ((y & 0xFFFF) != 0) {
    code = func_80166CB0(x >> 16, (y >> 16) + 1);
    if ((u8)(code + 0x10) < 2 || code == 0xF4 || (u8)(code + 0xA) < 2) {
      hit = 1;
    }
  }
  return hit;
}
