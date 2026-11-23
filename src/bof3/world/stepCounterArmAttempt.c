#include "bof3/bof3.h"

extern u16 D_80145AA8;
extern u16 D_801F4E04;
extern u8 D_801F4E0C;

/* @source 0x801F3658
 * @behavior Overlay step handler, entry 0x15 of the local handler table at
 *           0x801F3FF4 and entry 0x11 of the shifted table at 0x801F4004,
 *           which lists the same pointers four entries on: when the shared pad
 *           halfword D_80145AA8 reads exactly 0x40 it advances this overlay's
 *           own 16-bit counter D_801F4E04 by one, then always advances the
 *           attempt byte D_801F4E0C by one and branches on the truncated new
 *           value. Once that value has reached 0x1C (28) it stores 0x16 (22)
 *           into the byte at offset 0x01 of the work record published at the
 *           scratchpad pointer cell 0x1F800044 - under the table at 0x801F4004
 *           that index names the substate-8 reset handler
 *           setSubstate8AndClearWorkFlagsWorld01Area06813 - and below 0x1C it
 *           stores 0x12 (18), the countdown step
 *           countdownStepArmState14Or10ByPad40Else11World01Area06813 at the
 *           same index. Both arms read the pointer cell through the same base
 *           variable, so their byte stores merge into the single store that
 *           fills the return delay slot. The pad gate is whole-halfword
 *           equality, so the halfword load and its delay nop sit before the
 *           0x40 constant. Leaf with no frame: it takes no arguments, makes no
 *           call and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepCounterArmAttempt(void) {
  u8** slots;
  u8 attempt;

  if (D_80145AA8 == 0x40) {
    D_801F4E04 += 1;
  }
  attempt = D_801F4E0C + 1;
  D_801F4E0C = attempt;
  if (attempt >= 0x1Cu) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x16;
  } else {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x12;
  }
}
