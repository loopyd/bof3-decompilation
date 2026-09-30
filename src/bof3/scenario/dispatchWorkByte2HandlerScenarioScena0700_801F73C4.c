#include "bof3/bof3.h"

typedef void (*ScenarioScena0700WorkSubStateHandler)(void);

/* Overlay-local three-entry run of in-image code pointers based at 0x801FDD70,
 * selected by the work object's byte at offset 0x02: entry 0 is 0x801F7408,
 * entry 1 is 0x801F7480 and entry 2 is the exact sibling
 * resetWorkStateWhenWorkTableIdleScenarioScena0700_801F75C0, whose metadata
 * records this run and this dispatcher. The next label D_801FDD7C is the
 * non-pointer data 0x00000103, closing the run. The overlay's final Splat asm
 * block (the code-shaped boundary func_801FDD14) defines the label D_801FDD70;
 * the target map does not own it. */
extern ScenarioScena0700WorkSubStateHandler D_801FDD70[];

/* @source 0x801F73C4
 * @behavior Overlay state dispatcher for the work object's byte at offset 0x02:
 * it reads the scratchpad work object published by the pointer cell at
 * 0x1F800044 (`lui $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its
 * frame, takes that object's unsigned byte at offset 0x02
 * (`lbu $v0,0x2($v0)`), scales it by four (`sll $v0,$v0,2`) and invokes that
 * entry of the in-image code-pointer run based at 0x801FDD70 (`lui $at,%hi` /
 * `addu $at,$at,$v0` / `lw $v0,%lo($at)` / `jalr $v0`) with no arguments and no
 * result consumed, so the byte is the state index of the three-entry handler
 * window this dispatcher reads. It reads no other state and writes none; its
 * 0x18-byte frame exists only to keep $ra across the indirect call, whose delay
 * slot stays a nop, and the pointer-cell load precedes frame creation. 0x801F73C4
 * is entry 3 (the word at 0x801FDD20) of the ten-word work-object byte-1 handler
 * run based at 0x801FDD14 whose entry 0 is the exact sibling byte-2 dispatcher
 * dispatchWorkByte2HandlerScenarioScena0700_801F6E10 and whose entry 2 is
 * dispatchWorkByte2HandlerScenarioScena0700_801F700C, and that word is the only
 * image reference to the address, so the byte-1 dispatcher
 * dispatchWorkByte1HandlerScenarioScena0700_801F6DCC selects it when the work
 * object's byte 1 is 3. The run it dispatches is independently recorded by the
 * exact sibling entry 2 of that table,
 * resetWorkStateWhenWorkTableIdleScenarioScena0700_801F75C0, which states that
 * this address indexes 0x801FDD70 with the work-object byte at scratchpad
 * pointer slot 0x1F800044 + 0x02. The 68 bytes are instruction-for-instruction
 * identical to the byte-2 dispatchers at 0x801F700C (above), 0x801F6E10 (this
 * overlay), 0x801F71B8 and 0x801F73CC (scena12/00) and, in every instruction but
 * the table-address immediate word, to the byte-1 dispatchers 0x801F6DCC (this
 * overlay) and 0x801F6F48 (scena08/00). The name records the proven role (the
 * byte-offset-0x02 dispatcher the byte-1 run reaches at index 3) and the address
 * anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerScenarioScena0700_801F73C4(void) {
  D_801FDD70[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
