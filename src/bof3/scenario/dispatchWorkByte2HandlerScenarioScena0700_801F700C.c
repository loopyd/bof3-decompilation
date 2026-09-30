#include "bof3/bof3.h"

typedef void (*ScenarioScena0700WorkSubStateHandler)(void);

/* Overlay-local run of eight in-image code pointers based at 0x801FDD50,
 * selected by the work object's byte at offset 0x02: 0x801F7050, 0x801F70F8,
 * 0x801F71C0, 0x801F724C, 0x801F72E0, incrementWorkByte2ScenarioScena0700_801F7364,
 * incrementWorkByte2ScenarioScena0700_801F7384 and
 * invokeHelperScenarioScena0700_801F73A4. The overlay's final Splat asm block
 * (the code-shaped boundary func_801FDD14) labels the run D_801FDD50 and emits
 * the next run D_801FDD70 immediately after it, so the target map does not own
 * the address. */
extern ScenarioScena0700WorkSubStateHandler D_801FDD50[];

/* @source 0x801F700C
 * @behavior Overlay state dispatcher for the work object's byte at offset 0x02:
 * it reads the scratchpad work object published by the pointer cell at
 * 0x1F800044 (`lui $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its
 * frame, takes that object's unsigned byte at offset 0x02
 * (`lbu $v0,0x2($v0)`), scales it by four (`sll $v0,$v0,2`) and invokes that
 * entry of the in-image code-pointer run based at 0x801FDD50 (`lui $at,%hi` /
 * `addu $at,$at,$v0` / `lw $v0,%lo($at)` / `jalr $v0`) with no arguments and no
 * result consumed, so the byte is the state index of the handler window this
 * dispatcher reads. It reads no other state and writes none; its 0x18-byte
 * frame exists only to keep $ra across the indirect call, whose delay slot stays
 * a nop, and the pointer-cell load precedes frame creation. 0x801F700C is entry
 * 2 (the word at 0x801FDD1C) of the ten-word work-object byte-1 handler run
 * based at 0x801FDD14 whose entry 0 is the exact sibling byte-2 dispatcher
 * dispatchWorkByte2HandlerScenarioScena0700_801F6E10 and whose entry 3 is
 * 0x801F73C4, and that word is the only image reference to the address, so the
 * byte-1 dispatcher dispatchWorkByte1HandlerScenarioScena0700_801F6DCC selects
 * it when the work object's byte 1 is 2. The 68 bytes are
 * instruction-for-instruction identical to that entry-0 sibling
 * (dispatchWorkByte2HandlerScenarioScena0700_801F6E10, table 0x801FDD3C) and, in
 * every instruction but the table-address immediate word, to the byte-1
 * dispatcher dispatchWorkByte1HandlerScenarioScena0700_801F6DCC, differing only
 * in instruction 9, the table immediate. The name records the proven role (the
 * byte-offset-0x02 dispatcher the byte-1 run reaches at index 2) and the address
 * anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerScenarioScena0700_801F700C(void) {
  D_801FDD50[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
