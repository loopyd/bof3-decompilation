#include "bof3/bof3.h"

typedef void (*ScenarioScena0700WorkStateHandler)(void);

/* Overlay-local two-word run of in-image code pointers based at 0x801FDD48,
 * selected by the work object's byte at offset 2: its word 0 is 0x801F6F8C
 * (incrementWorkSetCounter, whose own lift
 * records the object fields that entry advances) and its word 1 is 0x801F6FB0,
 * whose Splat row read asm when this body was written. The next data label
 * D_801FDD50 at 0x801FDD50 opens the eight-word run that
 * callWorkByte2Step3 indexes with the same work
 * byte. The address sits inside the textbin block at 0x801FDD14, whose
 * generated undefined-symbol row binds it; the target map does not own it. */
extern ScenarioScena0700WorkStateHandler D_801FDD48[];

/* @source 0x801F6F48
 * @behavior Overlay handler dispatcher for the scratchpad work object's byte at
 * offset 2, entry 1 of the ten-word byte-1 handler run based at 0x801FDD14: it
 * loads the work-object pointer cell at 0x1F800044 (`lui $v0,0x1F80` then `lw
 * $v0,0x44($v0)`) before it creates its frame, takes that object's unsigned
 * byte at offset 2 (`lbu $v0,0x2($v0)`), scales it by four (`sll $v0,$v0,2`)
 * and invokes that slot of the overlay-local two-word in-image code-pointer run
 * based at 0x801FDD48 (`lui $at,%hi` / `addu $at,$at,$v0` / `lw $v0,%lo($at)`
 * / `jalr $v0`) with no arguments and no result consumed. That run holds
 * 0x801F6F8C at 0x801FDD48 and 0x801F6FB0 at 0x801FDD4C, and the next data
 * label D_801FDD50 follows it. It reads no state beyond that byte and writes
 * none; its 0x18-byte frame exists only to keep $ra across the indirect call,
 * whose delay slot stays a nop, and the pointer-cell load precedes frame
 * creation. The only word addressing 0x801F6F48 in this payload is the run
 * word at 0x801FDD18, entry 1 of the ten-word run that
 * callWorkByte1Step indexes with work byte 1,
 * so no in-image jal reaches the address. The 68 bytes are
 * instruction-for-instruction identical to callWorkByte2Step,
 * entry 0 of that same ten-word run, except for the table-address immediate word
 * (instruction 9), and that sibling's own gate measured it exact.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte2Step2(void) {
  D_801FDD48[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
