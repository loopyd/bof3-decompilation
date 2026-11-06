#include "bof3/bof3.h"

void func_801D1370(VECTOR* arg0, VECTOR* arg1, s32 arg2);

/* @source 0x801D1914
 * @behavior Overlay entry that draws a zigzag of five projected segments: it
 * seeds two 16-byte VECTOR locals at sp+0x10 and sp+0x20 with fixed fixed-point
 * coordinates and issues five calls to this overlay's two-point segment drawer
 * func_801D1370 (0x801D1370), rewriting one whole point between calls. The six
 * points are (0x308000, 0x718000, 0xFD000000), (0x308000, 0x718000,
 * 0xFE000000), (0x300000, 0x718000, 0xFE800000), (0x300000, 0x718000,
 * 0xFF000000), (0x308000, 0x718000, 0xFF800000) and (0x308000, 0x718000, 0), so
 * y never changes, x alternates between two values and z advances by 0x800000
 * per segment. The calls pass them in the order (1,2), (3,2), (3,4), (5,4),
 * (5,6): the same segment chain with the direction alternating so each call
 * starts at the point the previous one ended on. The caller's signed halfword
 * argument is sign-extended once at entry into the saved register the five calls
 * forward as their third argument; the callee's halfword stores fix that
 * argument's width and the caller therefore only proves the signed conversion.
 * The address has no in-payload `jal` and no word in the payload holds it, so
 * the scenario runtime reaches this entry from outside this payload. Takes one
 * signed halfword and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 * Decisive clean-C shape: every update writes the whole point, including the
 * unchanged y field. Writing only the fields that change kept both fixed
 * coordinates in caller-saved registers and dropped the four y stores (61/67,
 * 244 bytes, first difference at the entry saved-register group). With the y
 * store present at every update, 0x718000 and 0x308000 stay live across the
 * calls and take $s3 and $s4 as the original does; naming the three coordinates
 * as locals instead measured identical bytes, so the plain literals are kept.
 */
void drawZigzagOfFiveProjectedSegmentsScenarioSce15ef000_801D1914(s16 arg0) {
  VECTOR first;
  VECTOR second;

  first.vx = 0x308000;
  first.vy = 0x718000;
  first.vz = 0xFD000000;
  second.vx = 0x308000;
  second.vy = 0x718000;
  second.vz = 0xFE000000;
  func_801D1370(&first, &second, arg0);
  first.vx = 0x300000;
  first.vy = 0x718000;
  first.vz = 0xFE800000;
  func_801D1370(&first, &second, arg0);
  second.vx = 0x300000;
  second.vy = 0x718000;
  second.vz = 0xFF000000;
  func_801D1370(&first, &second, arg0);
  first.vx = 0x308000;
  first.vy = 0x718000;
  first.vz = 0xFF800000;
  func_801D1370(&first, &second, arg0);
  second.vx = 0x308000;
  second.vy = 0x718000;
  second.vz = 0;
  func_801D1370(&first, &second, arg0);
}
