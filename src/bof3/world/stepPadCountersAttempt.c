#include "bof3/bof3.h"

extern u16 D_80145AA8;
extern u16 D_801F4E04;
extern u8 D_801F4E0C;

/* @source 0x801F2F24
 * @behavior Overlay step handler, entry 3 of the local handler table at
 *           0x801F4004 that the dispatcher
 *           dispatchStateByte1Table4004World01Area06813 selects from with the
 *           work record byte at +1: while the shared pad halfword D_80145AA8
 *           reads exactly 0x10 it advances this overlay's own 16-bit counter
 *           D_801F4E04 by one, then always advances the attempt byte
 *           D_801F4E0C by one and branches on the truncated new value. Once
 *           that value has reached 0x1C (28) it stores 0x16 (22) into the
 *           record byte at +1 - entry 22 of the same table, the
 *           substate-8 reset handler - and below 0x1C it stores 4, entry 4 of
 *           that table. Both arms name the scratchpad pointer cell
 *           0x1F800044 through the same base variable, so the emitted byte
 *           stores coincide and the compiler merges them into the single
 *           store that follows the branch, each arm's state constant filling a
 *           branch delay slot. The pad gate is whole-halfword equality: the
 *           halfword load and its delay nop sit before the 0x10 constant, and
 *           the new attempt value is truncated with an AND and compared
 *           unsigned below 0x1C straight after its byte store. Leaf with no
 *           frame: it takes no arguments, makes no call and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepPadCountersAttempt(void) {
  u8** slots;
  u8 attempt;

  if (D_80145AA8 == 0x10) {
    D_801F4E04 += 1;
  }
  attempt = D_801F4E0C + 1;
  D_801F4E0C = attempt;
  if (attempt >= 0x1Cu) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x16;
  } else {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 4;
  }
}
