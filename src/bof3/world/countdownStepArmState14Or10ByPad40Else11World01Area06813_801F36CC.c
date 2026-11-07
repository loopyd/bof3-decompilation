#include "bof3/bof3.h"

extern u16 D_80145AA8;
extern u16 D_801F4E04;
extern u8 D_801F4E00;
extern u8 D_801F4E0C;

/* @source 0x801F36CC
 * @behavior Overlay step: counts this overlay's gate byte D_801F4E00 down by
 *           one, writes the truncated byte back and branches on the result.
 *           Each of the three arms selects the state index stored into the byte
 *           at offset 0x01 of the work record published at the scratchpad
 *           pointer cell 0x1F800044 - the byte the local dispatchers
 *           dispatchStateByte1Table3ff4World01Area06813 and
 *           dispatchStateByte1Table4004World01Area06813 index their handler
 *           tables with - and re-reads the pointer cell for its own store.
 *           When the countdown reaches zero the gate byte is cleared and 0x14
 *           is stored. Otherwise, while the shared pad halfword D_80145AA8
 *           reads exactly 0x40, the overlay's own 16-bit counter D_801F4E04 is
 *           advanced by one and the byte D_801F4E0C is cleared before 0x10 is
 *           stored; every other case stores 0x11. All three stores name the
 *           same record byte through the same base variable, so the emitted
 *           byte stores coincide and the compiler merges them into the single
 *           store that follows the branch, with each arm's index constant
 *           filling a branch delay slot. Leaf with no frame: it takes no
 *           arguments, makes no call and returns nothing. The D_801F3FF4
 *           handler table reaches it at entry 0x13.
 *           Shape evidence: the same-target sibling func_801F3578 carries the
 *           same countdown head and the same pad-0x40 arm (counter advance,
 *           D_801F4E0C clear, 0x10 state) with a different tail, and the exact
 *           closure incrementPadMatchCountAndArmState10World01Area06813
 *           supplies the counter/pointer/state statement order for that arm.
 *           No byte-identical sibling exists for the clone lever: the cell
 *           addresses and the 0x14/0x10/0x11 selection are encoded in this
 *           overlay's own instruction words.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownStepArmState14Or10ByPad40Else11World01Area06813_801F36CC(void) {
  u8** slots;
  u16 count;
  u8 gate;

  gate = D_801F4E00 - 1;
  D_801F4E00 = gate;
  if (gate == 0) {
    slots = SPAD_PTR_TABLE(u8);
    D_801F4E00 = 0;
    slots[0x11][0x01] = 0x14;
  } else if (D_80145AA8 == 0x40) {
    count = D_801F4E04;
    slots = SPAD_PTR_TABLE(u8);
    D_801F4E0C = 0;
    D_801F4E04 = count + 1;
    slots[0x11][0x01] = 0x10;
  } else {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x11;
  }
}
