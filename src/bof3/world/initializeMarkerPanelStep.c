#include "bof3/world/area03004_internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @behavior Call applyMainLookupRecord(0x47), initialize the marker panel work fields,
 * clear its countdown and level bytes, and advance handler byte 3. The
 * following marker-panel step consumes byte 6 as its vertical offset.
 * @source 0x801D6044
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeMarkerPanelStep(void) {
  volatile u8* panelWork;
  volatile u8* levelWork;
  volatile u8* dispatchWork;

  WORLD00_AREA030_INIT_PANEL_WORK(
      ((volatile struct scratchpad_state*)0x1f800000u)->scratch,
      panelWork, levelWork, 0xf0, 0xc2);
  WORLD00_AREA030_CLEAR_PANEL_LEVELS_AND_COUNTERS(
      levelWork, ((volatile struct scratchpad_state*)0x1f800000u)->scratch);
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[6] = 0;
  dispatchWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  {
    u8 workStep = dispatchWork[3];
    *(u8*)(dispatchWork + 3) = workStep + 1;
  }
}
