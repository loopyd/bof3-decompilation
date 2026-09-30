#include "bof3/bof3.h"

void clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC(void);

/* @source 0x801F7AC8
 * @behavior Entry 0 of this overlay's three-word work-object handler table at
 * 0x801FA1E4 ({0x801F7AC8, 0x801F7B00, 0x801F7BC0}), which the per-frame
 * dispatcher func_801F7A84 indexes with byte 0x1 of the scratch work object
 * published at pointer slot 0x1F800044 (it reads that byte, shifts it left by
 * two and `jalr`s the selected word) and calls with no arguments: it clears the
 * whole 0x800E4800 record table's byte-0x25 field through
 * clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC (jal at 0x801F7AD0)
 * and then advances that work byte by one, so the next frame dispatches entry 1
 * (0x801F7B00). The work byte is reloaded after the call because the callee
 * clobbers the registers. It takes no arguments, returns nothing, reads no
 * state other than the pointer cell and writes only that work byte; the
 * 0x18-byte frame exists only to hold $ra across the call, whose delay slot
 * stays a nop because the callee takes no argument. The address's only image
 * word is 0x801FA1E4, word 0 of that pointer run, so the overlay reaches this
 * handler only indirectly. Its 0x38 bytes are instruction-for-instruction
 * identical to the exact sibling
 * queueCue209AndAdvanceWorkByte1ScenarioScena0300_801F7E34 (0x801F7E34 in
 * emi/scenario/scena03/00): call, reload, increment the same work byte; only
 * the call target and that call's delay slot differ, and that sibling owns its
 * own address, boundary, map row and source.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkTableByte25AndAdvanceWorkByte1ScenarioScena0400_801F7AC8(void) {
  u8* work;

  clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC();
  work = SPAD_PTR_SLOT(u8, 0x44u);
  work[1]++;
}
