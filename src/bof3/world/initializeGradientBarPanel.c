#include "bof3/world/area03004_internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @behavior Call applyMainLookupRecord(0x47), initialize the gradient-bar panel fields,
 * clear its level and countdown bytes, set width word 0x0C to 0 and
 * width-delta word 0x18 to 4, then advance handler byte 3. Preserve both
 * byte-6 clears and the separate pointer captures.
 * @source 0x801D5C8C
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeGradientBarPanel(void) {
  volatile u8* panelWork;
  volatile u8* levelWork;
  volatile u8* barWork;
  volatile u8* dispatchWork;

  WORLD00_AREA030_INIT_PANEL_WORK(
      ((volatile struct scratchpad_state*)0x1f800000u)->scratch,
      panelWork, levelWork, 0xec, 0xa2);
  WORLD00_AREA030_CLEAR_PANEL_LEVELS_AND_COUNTERS(
      levelWork, ((volatile struct scratchpad_state*)0x1f800000u)->scratch);
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[6] = 0;
  barWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  barWork[6] = 0;
  dispatchWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(volatile u32*)(barWork + 0xc) = 0;
  *(volatile u32*)(barWork + 0x18) = 4;
  {
    u8 workStep = dispatchWork[3];
    *(u8*)(dispatchWork + 3) = workStep + 1;
  }
}
