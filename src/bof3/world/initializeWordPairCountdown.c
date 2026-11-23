#include "bof3/world/area03004_internal.h"

struct scratchpad_state {
  u8           pad[0x44];
  volatile u8* scratch;
};

/* @source 0x801D6414
 * @behavior Call applyMainLookupRecord(0x2B), set work byte 0x24 to 0x80, clear bytes
 * 0x29 and 0x2A, and store 2 in byte 0x48. Clear bytes 0x5D through 0x5F,
 * store 0x8000 in words 0x40 and 0x44, set countdown 64 and clear byte 6. Call
 * selectActiveWorkSequence(3), then advance a fresh handler byte 3.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initializeWordPairCountdown(void) {
  volatile u8* levelClearWork;
  volatile u8* dispatchWork;
  volatile u8* wordPairWork;
  volatile u8* setupWork;
  volatile u8* byteSixWork;

  applyMainLookupRecord(0x2b);
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x24] = 0x80;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x29] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x2a] = 0;
  setupWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  setupWork[0x48] = 2;
  wordPairWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  wordPairWork[0x5d] = 0;
  levelClearWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(volatile u32*)(wordPairWork + 0x40) = 0x8000;
  *(volatile u32*)(wordPairWork + 0x44) = 0x8000;
  levelClearWork[0x5e] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[0x5f] = 0;
  ((volatile struct scratchpad_state*)0x1f800000u)->scratch[9] = 0x40;
  byteSixWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  *(u8*)(byteSixWork + 6) = 0;
  selectActiveWorkSequence(3);
  dispatchWork = ((volatile struct scratchpad_state*)0x1f800000u)->scratch;
  {
    u8 handlerIndex = dispatchWork[3];
    *(u8*)(dispatchWork + 3) = handlerIndex + 1;
  }
}
