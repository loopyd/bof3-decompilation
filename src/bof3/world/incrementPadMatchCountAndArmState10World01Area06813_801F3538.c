#include "bof3/bof3.h"

extern u16 D_80145AA8;
extern u16 D_801F4E04;

/* @source 0x801F3538
 * @behavior Overlay step: while the shared pad halfword D_80145AA8 reads
 *           exactly 0x40 it advances this overlay's own 16-bit counter
 *           D_801F4E04 by one and then stores 0x10 into the state byte at
 *           offset 0x01 of the work record published at the scratchpad
 *           pointer cell 0x1F800044 - the byte the local dispatchers
 *           dispatchStateByte1Table3ff4World01Area06813 and
 *           dispatchStateByte1Table4004World01Area06813 index their handler
 *           tables with. The counter load comes first and the record pointer
 *           load sits between that load and the increment, so the increment
 *           fills the pointer load's delay slot; the counter store then
 *           precedes the 0x10 constant and its byte store. The test is
 *           whole-halfword equality with no mask instruction, and the nop in
 *           the branch delay slot is the empty-arm join. Leaf with no frame:
 *           it takes no arguments, makes no call and returns nothing.
 *           Shape evidence: the same-target siblings func_801F2F24 (gate
 *           0x10, same counter, no state store) and func_801F3578 (gate
 *           0x40, same counter, an extra D_801F4E0C clear and a different
 *           state index) supply the statement order; none of the 34 world01
 *           slot-13 payloads holds these 0x40 bytes at offset 0x938, and the
 *           counter immediate is this overlay's own, so no byte-identical
 *           sibling exists for the clone lever.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementPadMatchCountAndArmState10World01Area06813_801F3538(void) {
  u8** slots;
  u16 count;

  if (D_80145AA8 == 0x40) {
    count = D_801F4E04;
    slots = SPAD_PTR_TABLE(u8);
    D_801F4E04 = count + 1;
    slots[0x11][0x01] = 0x10;
  }
}
