#include "bof3/bof3.h"

void clearWorkTableAt800E4800ScenarioScena0800_801F8AE8(void);

/* @source 0x801F79B0
 * @behavior Overlay work-state handler: it calls the table-clearing helper at
 * 0x801F8AE8, which zeroes the lead byte of each of the 0x8 entries of the
 * 0x1C-byte-stride record table at 0x800E4800, and then re-arms the step: it
 * reloads the scratchpad work object published at pointer slot 0x1F800044 + 0x44
 * (the call clobbers the registers, so the cell is read again after it), stores
 * 0x40 into that object's halfword at offset 0x5A and increments its byte at
 * offset 0x02 by one. The neighbour at 0x801F79EC (the next entry of the same
 * handler run) is the evidence for the halfword: it reloads the same 0x5A
 * halfword each frame, decrements it by one and increments the object's byte at
 * offset 0x02 once the decremented value reads zero, so 0x5A is the countdown
 * that byte 2 advances on. It takes no arguments, returns nothing, reads only
 * the pointer slot and writes only the two object fields plus what the callee
 * owns; its 0x3C bytes keep $ra across the single call, so the frame is 0x18
 * bytes and the call's delay slot stays a nop (nothing is live into it). The
 * address is entry 17 (work-object byte 1 = 0x11) of this overlay's 51-entry
 * per-frame handler run at 0x801FE894, the table that
 * dispatchWorkByte1HandlerScenarioScena0800_801F6F48 indexes with the scratch
 * work object's byte 1.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableIncrementWorkByte2SetCountdownScenarioScena0800_801F79B0(void) {
  u8* work;
  u8 step;

  clearWorkTableAt800E4800ScenarioScena0800_801F8AE8();
  work = SPAD_PTR_SLOT(u8, 0x44u);
  step = work[2];
  *(u16*)(work + 0x5A) = 0x40;
  work[2] = (u8)(step + 1u);
}
