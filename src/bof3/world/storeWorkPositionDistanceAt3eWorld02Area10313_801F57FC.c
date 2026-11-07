#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Naming the cell keeps the original's
 * per-access re-materialization (lui $vN,0x1F80 / lw $vN,68($vN)); the bare
 * constant address makes the compiler CSE 0x1F800044 into one saved register
 * and load through offset zero instead.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

s16 func_8015477C(s32 coord_x, s32 coord_y);

/* @source 0x801F57FC
 * @behavior Overlay step handler: hands the fixed-point position pair of the
 *           work record published through the scratchpad pointer cell
 *           0x1F800044, its x word at 0x34 and its y word at 0x38, to the
 *           shared func_8015477C distance helper, re-reads the cell because
 *           the helper may republish it, and stores the returned signed
 *           distance in the record's halfword at 0x3E. The 0x18-byte frame
 *           only keeps $ra across the call. Takes no arguments, returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 * Two pointer locals, not one: the call group's cell read stays in $v0 while
 * the store group's takes $v1, and one variable reused across both groups
 * becomes a single pseudo that takes $v1 throughout.
 */
void storeWorkPositionDistanceAt3eWorld02Area10313_801F57FC(void) {
  u8* work;
  u8* target;
  s16 distance;

  work = D_1F800044;
  distance = func_8015477C(*(s32 *)(work + 0x34), *(s32 *)(work + 0x38));
  target = D_1F800044;
  *(s16 *)(target + 0x3E) = distance;
}
