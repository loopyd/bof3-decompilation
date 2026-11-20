#include "bof3/world/area03004_internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @source 0x801D5B70
 * @behavior Call func_8014DD3C(0x47), initialize panel work with byte 0x24 = 0x80, byte
 * 0x29 = 1, bytes 0x2A and 0x48 = 0, and halfwords 0x2E = 0xE8 and 0x30 =
 * 0xC2. Clear bytes 9, 0xA and 0x5D through 0x5F, call func_8014D6B8(10), then
 * decrement a fresh handler byte 3 modulo 256.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializePanelWorkAndDecrementHandlerWorld00Area03004(void) {
  volatile u8* panelWordWork;
  volatile u8* countdownWork;
  volatile u8* tailClearWork;
  volatile u8* dispatchWork;

  WORLD00_AREA030_INIT_PANEL_WORK(
      ((volatile struct scratchpad_state*)0x1f800000u)->scratch,
      panelWordWork, countdownWork, 0xe8, 0xc2);
  countdownWork[9] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[10] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5d] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5e] = 0;
  tailClearWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(u8*)(tailClearWork + 0x5f) = 0;
  func_8014D6B8(0xa);
  dispatchWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  {
    u8 handlerIndex = dispatchWork[3];
    *(u8*)(dispatchWork + 3) = handlerIndex - 1;
  }
}
