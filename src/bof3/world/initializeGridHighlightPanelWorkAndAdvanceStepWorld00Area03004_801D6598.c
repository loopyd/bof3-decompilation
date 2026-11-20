#include "bof3/world/area03004_internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @behavior Call func_8014DD3C(0x47), initialize the grid-highlight panel fields and
 * clear its phase, countdown and row bytes. Advance handler byte 3 before
 * calling func_8014D6B8(3).
 * @source 0x801D6598
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeGridHighlightPanelWorkAndAdvanceStepWorld00Area03004(void) {
  volatile u8* panelWork;
  volatile u8* levelWork;
  volatile u8* dispatchWork;

  WORLD00_AREA030_INIT_PANEL_WORK(
      ((volatile struct scratchpad_state*)0x1f800000u)->scratch,
      panelWork, levelWork, 0xca, 0xba);
  WORLD00_AREA030_CLEAR_PANEL_LEVELS_AND_COUNTERS(
      levelWork, ((volatile struct scratchpad_state*)0x1f800000u)->scratch);
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[11] = 0;
  dispatchWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  {
    u8 workStep = dispatchWork[3];
    *(u8*)(dispatchWork + 3) = workStep + 1;
  }
  func_8014D6B8(3);
}
