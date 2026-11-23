#include "bof3/bof3.h"

extern u8* D_1F800044;

void func_801F7660(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* @source 0x801F745C
 * @behavior Work-byte-1 handler reached only through the overlay's handler
 * table D_801FD5A8 (word 1), so nothing in the payload calls it directly. It
 * reads the work-object pointer cell at scratchpad 0x1F800044 before the frame
 * is created and hands the object's cursor triple at offsets 0x34, 0x38 and
 * 0x3C plus its unsigned 16-bit word at offset 0x0C to the overlay-local
 * drawing helper at 0x801F7660, which is raw here and whose own body emits the
 * primitives. It then re-reads the same pointer cell, advances the 32-bit
 * counter at offset 0x0C of that object by 0x10 and, once the new value reaches
 * 0x501, increments the object's unsigned byte at offset 0x01, which is the
 * byte the overlay's dispatcher indexes the handler table with. It takes no
 * arguments and returns nothing; the 0x18-byte frame only keeps $ra across the
 * helper call. No other span in the tree disassembly carries the 0x501
 * immediate, so this body was seeded from its own bytes plus the work-byte-1
 * and counter spellings of the two same-target handlers whose sources measure
 * exact, incrementClearCounterScenario (0x801F743C)
 * and incrementWorkByte1SetCounterScenarioScena0500_801F7530 (0x801F7530).
 * @status exact
 * @match 100.00
 * @residual none
 */
void renderCursorTickProgress(
    void) {
  u8* work;
  u8* next;
  s32 progress;

  work = D_1F800044;
  func_801F7660(*(s32*)(work + 0x34), *(s32*)(work + 0x38),
      *(s32*)(work + 0x3c), *(u16*)(work + 0x0c));
  next = D_1F800044;
  progress = *(s32*)(next + 0x0c) + 0x10;
  *(s32*)(next + 0x0c) = progress;
  if (progress >= 0x501) {
    next[1]++;
  }
}
