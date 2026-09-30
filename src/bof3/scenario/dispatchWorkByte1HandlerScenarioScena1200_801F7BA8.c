#include "bof3/bof3.h"

typedef void (*ScenarioScena1200WorkStateHandler)(void);

/* Code-pointer run based at 0x801FD198, selected by the work object's byte at
 * offset 0x01: entry 0 is 0x801F7BEC, which advances that byte, and entry 1 is
 * 0x801F7C24. The address sits inside the overlay's final asm block, which
 * defines the label D_801FD198; the target map does not own it. */
extern ScenarioScena1200WorkStateHandler D_801FD198[];

/* @source 0x801F7BA8
 * @behavior Overlay per-frame state dispatcher for the work object's byte at
 * offset 0x01: it reads the scratchpad work object published by the pointer
 * cell at 0x1F800044, takes that object's unsigned byte at offset 0x01, scales
 * it by four and invokes that entry of the in-image code-pointer run based at
 * 0x801FD198 with no arguments. Entry 0 of that run is 0x801F7BEC
 * (clearFieldAdvanceStateAndQueueCue215ScenarioScena1200_801F7BEC), which adds
 * one to exactly this byte and queues cue 0x215, and entry 1 is 0x801F7C24, so
 * that byte is the state index of the window this dispatcher reads. It reads no
 * other state and writes none; the pointer cell is loaded before the 0x18-byte
 * frame is created and that frame only keeps $ra across the indirect call. The
 * table address is built as `lui $at, %hi(table)` / `addu $at, $at, index * 4` /
 * `lw $v0, %lo(table)($at)`. The 68 bytes are instruction-for-instruction
 * identical to the byte-1 dispatchers at 0x801F7174
 * (dispatchWorkByte1HandlerScenarioScena1200_801F7174, whose window is
 * 0x801FD178 and which reaches 0x801F7BEC as that window's entry 8, where the
 * byte moves from 8 to 9), 0x801F6F48 of emi/scenario/scena08/00 and 0x801F6F38
 * and 0x801F77F8 of emi/scenario/scena15/00, differing only in that
 * table-address word (compared directly in the shipped payloads). No in-image
 * jal and no pointer word targets 0x801F7BA8, so the caller that selects this
 * window is outside the overlay. The name records the proven role and the
 * address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena1200_801F7BA8(void) {
  D_801FD198[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
