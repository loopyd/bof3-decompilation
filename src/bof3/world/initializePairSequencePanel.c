#include "bof3/world/area03004_internal.h"

/* @source 0x801D5A48
 * @behavior Call applyMainLookupRecord(0x21), initialize the panel fields and both
 * byte-6 clears, seed word 0x0C = 0 and word 0x18 = 4, and call
 * selectActiveWorkSequence(0). Advance handler byte 3 to the pair-sequence and
 * queue step.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializePairSequencePanel(void) {
  u8* panelWork;
  u8* levelWork;
  u8* seedWork;

  applyMainLookupRecord(0x21);
  D_1F800044[0x24] = 0x80;
  D_1F800044[0x29] = 1;
  D_1F800044[0x2a] = 1;
  panelWork = D_1F800044;
  panelWork[0x48] = 0;
  levelWork = D_1F800044;
  *(u16*)(panelWork + 0x2e) = 0xa2;
  *(u16*)(panelWork + 0x30) = 0xc2;
  WORLD00_AREA030_CLEAR_PANEL_LEVELS_AND_COUNTERS(levelWork, D_1F800044);
  D_1F800044[6] = 0;
  seedWork = D_1F800044;
  *(u32*)(seedWork + 0xc) = 0;
  *(u32*)(seedWork + 0x18) = 4;
  seedWork[6] = 0;
  selectActiveWorkSequence(0);
  {
    u8* dispatchWork = D_1F800044;
    u8 workStep = dispatchWork[3];
    dispatchWork[3] = workStep + 1;
  }
}
