#include "bof3/bof3.h"

extern u16 D_8014932A;
extern u8 D_80149333;

/* @source 0x801F3D0C
 * @behavior Overlay counter step: reads the shared halfword counter at
 * 0x8014932A, marks the counter active by storing 2 into the area status byte
 * at 0x80149333, then writes the counter back advanced by 0x14. Its retreat
 * half, the adjacent retreatCounter2By20World02Area08013_801F3CE4 at
 * 0x801F3CE4 in this same payload, carries the identical 40-byte body with the
 * step immediate negated (0x0014 against 0xFFEC), which was the only word the
 * two spans differed in.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceCounter2By20World02Area08013_801F3D0C(void) {
  u16 count_;

  count_ = D_8014932A;
  D_80149333 = 2;
  D_8014932A = (u16)(count_ + 0x14);
}
