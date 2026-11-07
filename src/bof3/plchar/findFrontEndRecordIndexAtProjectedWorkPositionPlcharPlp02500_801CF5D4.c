#include "bof3/bof3.h"

/* Front-end record table of the shared main-executable data segment: 20 rows of
 * 0x74 bytes based at 0x80143FC8, with the row byte 0 occupancy flag at
 * 0x80143FC8, the row byte 5 handler selector at 0x80143FCD, the row word x at
 * 0x80143FFC and the row word y at 0x80144000. This body also scans that table
 * by address, so the base keeps a symbol of its own beside the field bases. The
 * original re-materialises `lui at,0x8014` and adds the scaled row offset for
 * every access, so each address-derived symbol stays separate.
 * @source 0x80143FC8 @kind unknown
 */
extern u8 D_80143FC8[];
/* @source 0x80143FCD @kind unknown */
extern u8 D_80143FCD[];
/* @source 0x80143FFC @kind unknown */
extern u8 D_80143FFC[];
/* @source 0x80144000 @kind unknown */
extern u8 D_80144000[];

/* Movement offset tables of the shared main-executable data section: eight-byte
 * records holding the first-coordinate offset at 0x80181B94 and the
 * second-coordinate offset at 0x80181B98, selected by the active work area's
 * route byte at offset 0x08. The original scales that byte by eight once, which
 * is element index route * 2, and adds it to each table base, so both bases stay
 * their own symbol.
 * @source 0x80181B94 @kind table
 */
extern s32 D_80181B94[];
/* @source 0x80181B98 @kind table */
extern s32 D_80181B98[];

/* Pointer cell of the shared main-executable record whose state byte 0x79 and
 * byte 0x12C gate this scan. Its role beyond those two bytes is not
 * established, so it stays address-anchored.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Scratchpad pointer cell 0x1F800044 holding the active work area. Every use
 * re-evaluates the cell: the three reads before the scan fold into one load
 * because no call separates them, while the read after the gate is a fresh load
 * of its own. The original materialises that address with `lui` plus an offset
 * load per evaluation instead of keeping it in a saved register, so the source
 * keeps the named-symbol spelling the sibling overlays of this domain bind.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Shared position gate of the main executable, outside this payload: it reports
 * one when the supplied point sits inside the supplied record's range. The
 * original zero-extends the result byte before testing it, so this overlay sees
 * the unsigned return type. */
u8 func_801BDD58(s32 x, s32 y, s16 reference, s32 range, void* work);

/* @source 0x801CF5D4
 * @behavior Finds the index of the first front-end record whose stored position
 *           matches the projected position of the active work area. The body
 *           first gates on the record published at 0x80146250: its state byte
 *           0x79 must read 2 and bit 0 of its byte 0x12C must be clear,
 *           otherwise the scan is skipped and the body answers -1. It then reads
 *           the active work area through the scratchpad pointer cell 0x1F800044
 *           at each use, taking that record's route byte at 0x08, the word at
 *           0x34 as the first coordinate and the word at 0x38 as the second, and
 *           projects them: the route byte, scaled by eight, selects an
 *           eight-byte movement offset record whose first offset at 0x80181B94
 *           and second offset at 0x80181B98 are added without doubling. The
 *           projected pair stays in registers across the whole scan. The scan
 *           itself walks the twenty 0x74-byte front-end rows of 0x80143FC8 by
 *           address offset and considers only rows whose byte 0 is nonzero and
 *           whose byte 5 equals 0x30; for those it passes the projected pair,
 *           the work area's halfword at 0x3E as a signed reference with no
 *           bias, the range 1 and the row's own address to the shared gate
 *           func_801BDD58, and once that gate reports a nonzero byte it
 *           re-reads the work area through the scratchpad cell, reloads the row
 *           word x at 0x80143FFC and the row word y at 0x80144000 beside the
 *           work area's words at 0x34 and 0x38, and returns the row index as a
 *           signed byte when either coordinate matches. Rows that fail any test
 *           are stepped over and 0x74 is added to the row offset; the scan stops
 *           after twenty rows and answers -1. The original tests the row
 *           conditions before the gate, loads the two work-area words before it
 *           scales the route byte, and keeps the row offset in a saved register.
 *           Its only caller is the still-`asm` row func_801CF37C of this same
 *           payload, which takes the low byte of the result, treats 0xFF as no
 *           match and otherwise feeds the value to the two position tests of
 *           this target as their record slot, so the return type is a signed
 *           byte with -1 as its no-match answer. Its 356-byte body is
 *           byte-identical to the still-`asm` row func_801CF5D4 in PLP026 (cmp
 *           of the two payload spans, all 356 bytes equal), so that twin is
 *           shape evidence and not a clone; the shape was seeded from this
 *           target's exact lift of 0x801CF808, which projects the same pair
 *           before its scan and passes the same gate, and from the exact
 *           game00 lift of 0x801BDE14, which walks a record table with a
 *           post-increment counter and returns `(s8)i` on a gate match and -1
 *           otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
s8 findFrontEndRecordIndexAtProjectedWorkPositionPlcharPlp02500_801CF5D4(void) {
  u8* record;
  u8* work;
  s32 i;
  s32 row;
  s32 x;
  s32 y;
  u8 route;

  record = D_80146250;
  if (record[0x79] != 2) {
    return -1;
  }
  if (record[0x12C] & 1) {
    return -1;
  }
  i = 0;
  row = 0;
  route = D_1F800044[8];
  x = *(s32*)(D_1F800044 + 0x34) + D_80181B94[route * 2];
  y = *(s32*)(D_1F800044 + 0x38) + D_80181B98[route * 2];
  do {
    if (D_80143FC8[row] != 0 && D_80143FCD[row] == 0x30) {
      if (func_801BDD58(x, y, *(s16*)(D_1F800044 + 0x3E), 1,
                        D_80143FC8 + row) != 0) {
        work = D_1F800044;
        if (*(s32*)(D_80143FFC + row) == *(s32*)(work + 0x34) ||
            *(s32*)(D_80144000 + row) == *(s32*)(work + 0x38)) {
          return (s8)i;
        }
      }
    }
    i++;
    row += 0x74;
  } while (i < 20);
  return -1;
}
