#include "bof3/bof3.h"

extern u8 D_80143BB0;

/* @source 0x801F2E2C
 * @behavior Overlay mode gate: reads the shared front-end mode byte
 * D_80143BB0 and, unless that byte reads exactly 5, loads the work-object
 * pointer published at the scratchpad cell 0x1F800044 and stores 1 into that
 * object's byte at offset 0x02. The compared constant 5 is materialised before
 * the branch and the stored 1 fills that branch's delay slot, and the pointer
 * cell is read only on the storing path, so the original keeps no local and no
 * frame. Offset 0x02 is the work-object byte the mode reset clears for a fresh
 * mode-1 pass, and D_80143BB0 is the front-end mode byte the scenario loader
 * waits against its ready_phase 5; the name stays anchored on the byte it sets
 * rather than asserting a gameplay role for it. It takes no arguments, makes no
 * call, returns nothing and reads no state besides D_80143BB0 and the cell.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setScratchWorkByte2WhenFrontEndModeNot5World03Area14514_801F2E2C(void) {
  u8** slots;

  if (D_80143BB0 != 5) {
    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][0x02] = 1;
  }
}
