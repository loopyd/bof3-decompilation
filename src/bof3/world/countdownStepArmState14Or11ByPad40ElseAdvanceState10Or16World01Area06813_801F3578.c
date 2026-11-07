#include "bof3/bof3.h"

extern u16 D_80145AA8;
extern u16 D_801F4E04;
extern u8 D_801F4E00;
extern u8 D_801F4E0C;
extern u32 D_80146865;

/* @source 0x801F3578
 * @behavior Overlay step: counts this overlay's gate byte D_801F4E00 down by
 *           one, writes the truncated byte back and branches on the result.
 *           Each arm selects the state index stored into the byte at offset
 *           0x01 of the work record published at the scratchpad pointer cell
 *           0x1F800044 - the byte the local dispatchers
 *           dispatchStateByte1Table3ff4World01Area06813 and
 *           dispatchStateByte1Table4004World01Area06813 index their handler
 *           tables with - and re-reads that pointer cell for its own store.
 *           When the countdown reaches zero, 0x14 is stored. Otherwise, while
 *           the shared pad halfword D_80145AA8 reads exactly 0x40, the
 *           overlay's own 16-bit counter D_801F4E04 is advanced by one, the
 *           byte D_801F4E0C is cleared, 0x11 is stored and the shared byte
 *           D_80146865 is advanced by one; every other case advances
 *           D_801F4E0C by one and stores 0x10 while the new byte is below
 *           0x1C, else 0x16.
 *           The D_80146865 cell is the shared story-progress byte that follows
 *           0x80146864. Declaring it wide and advancing only its low byte
 *           through a u8 cast makes the address be computed once into a
 *           register, which is the same spelling the exact sibling
 *           advanceScenarioProgressWhenWorkByte79ClearWorld02Area10313 uses
 *           for 0x80146864; the byte form folds the load and re-materialises
 *           $at for the store instead.
 *           Shape evidence: the same-target exact sibling
 *           countdownStepArmState14Or10ByPad40Else11World01Area06813 supplies
 *           the countdown head and the counter/pointer/state order of the
 *           pad-0x40 arm, and the unlifted neighbour func_801F3658 carries the
 *           same 0x1C range test with a 0x12/0x16 pair. No byte-identical
 *           sibling exists for the clone lever: this overlay's own cell
 *           addresses sit in the instruction words, and no other EMI payload
 *           holds these 0xE0 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void countdownStepArmState14Or11ByPad40ElseAdvanceState10Or16World01Area06813_801F3578(void) {
  u8** slots;
  u16 count;
  u8 gate;
  u8 state;

  gate = D_801F4E00 - 1;
  D_801F4E00 = gate;
  if (gate == 0) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x01] = 0x14;
  } else if (D_80145AA8 == 0x40) {
    count = D_801F4E04;
    slots = SPAD_PTR_TABLE(u8);
    D_801F4E0C = 0;
    D_801F4E04 = count + 1;
    slots[0x11][0x01] = 0x11;
    *(u8*)&D_80146865 += 1;
  } else {
    state = D_801F4E0C + 1;
    D_801F4E0C = state;
    if (state >= 0x1Cu) {
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11][0x01] = 0x16;
    } else {
      slots = SPAD_PTR_TABLE(u8);
      slots[0x11][0x01] = 0x10;
    }
  }
}
