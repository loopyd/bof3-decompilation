#include "bof3/bof3.h"

s32 func_801782FC(s32 arg0);
void func_801690EC(s32 arg0, s32 arg1, s32 arg2);

/* @source 0x801F9E70
 * @behavior Sets four ids through the shared three-argument setter
 * func_801690EC (0x801690EC): the ids 0x11, 0x12 and 0x13 take one magnitude and
 * the id 0x10 takes a second one, each magnitude passed as both the second and
 * the third argument. Both magnitudes come from the shared 12-bit rotation
 * helper func_801782FC (0x801782FC). The first halves the incoming 16-bit angle
 * 0x4C0 lower, takes the helper result as its own positive value (a negative
 * result is negated) and doubles it as a signed 16-bit quantity; the second does
 * the same with the same angle 0xCC0 lower, so the function publishes one signed
 * 16-bit magnitude from each of the two angle offsets. The argument is masked
 * with `andi 0xFFFF` once on entry and kept in a saved register across all five
 * calls because both offsets are derived from it, and the 0x20-byte frame keeps
 * $ra, $s0 and $s1. It takes that one 16-bit angle, returns nothing and reads no
 * memory of its own. No word and no jal in this overlay names 0x801F9E70, so its
 * caller lies outside the payload and the argument role stays structural.
 *
 * Matching note: the positive magnitude and its 16-bit doubling are one
 * expression, `(__builtin_abs(rotation) << 17) >> 16` as written below.
 * Splitting the abs and the shift into separate statements, or
 * writing the abs as a ternary or an if/else, left the shift duplicated into the
 * branch arms, the branch on the copied register with an empty delay slot, and
 * the id immediate after the join instead of hoisted above the branch; the
 * single builtin expression reproduces the original's `bgez`/delay-slot copy/
 * in-place `negu`/`sll`/`sra` group with the id immediate ahead of the branch.
 * The two published magnitudes are separate locals: one value shared by both
 * halves put the second magnitude in a saved register and cost an extra copy.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSharedIds11To13And10FromAngleOffsetsScenarioScena1300_801F9E70(s32 arg0) {
  s32 angle;
  s32 rotation;
  s32 tripletMagnitude;
  s32 finalMagnitude;

  angle = arg0 & 0xFFFF;
  rotation = func_801782FC((angle - 0x4C0) >> 1);
  tripletMagnitude = (__builtin_abs(rotation) << 17) >> 16;
  func_801690EC(0x11, tripletMagnitude, tripletMagnitude);
  func_801690EC(0x12, tripletMagnitude, tripletMagnitude);
  func_801690EC(0x13, tripletMagnitude, tripletMagnitude);
  rotation = func_801782FC((angle - 0xCC0) >> 1);
  finalMagnitude = (__builtin_abs(rotation) << 17) >> 16;
  func_801690EC(0x10, finalMagnitude, finalMagnitude);
}
