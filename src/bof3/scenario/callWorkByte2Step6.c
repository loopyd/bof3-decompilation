#include "bof3/bof3.h"

typedef void (*ScenarioScena0700WorkSubStateHandler)(void);

/* Overlay-local run of two in-image code pointers based at 0x801FDDAC, selected
 * by the work object's byte at offset 0x02: 0x801F78B0 and 0x801F7960, with the
 * next data label workByte2HandlerTable closing the run at 0x801FDDB4. That
 * following label is a separate three-word code-pointer run (0x801F7A30,
 * 0x801F7AB0 and stateClearCallbackFlags) which entry 6 of
 * the same byte-1 run, callWorkByte2Step7, indexes; this dispatcher's run ends before
 * it. The address sits inside the overlay's final Splat asm block (the
 * code-shaped boundary func_801FDD14), which defines the label; the target map
 * does not own it. */
extern ScenarioScena0700WorkSubStateHandler D_801FDDAC[];

/* @source 0x801F786C
 * @behavior Overlay state dispatcher for the work object's byte at offset 0x02:
 * it loads the work-object pointer cell at 0x1F800044 (`lui $v0,0x1F80` then
 * `lw $v0,0x44($v0)`) before it creates its frame, takes that object's unsigned
 * byte at offset 0x02 (`lbu $v0,0x2($v0)`), scales it by four (`sll $v0,$v0,2`)
 * and invokes that entry of the in-image code-pointer run based at 0x801FDDAC
 * (`lui $at,%hi` / `addu $at,$at,$v0` / `lw $v0,%lo($at)` / `jalr $v0`) with no
 * arguments and no result consumed, so that byte is the state index of the
 * handler window this dispatcher reads. It reads no other state and writes none;
 * its 0x18-byte frame exists only to keep $ra across the indirect call, whose
 * delay slot stays a nop, and the pointer-cell load precedes frame creation.
 * 0x801F786C is entry 5 (the word at 0x801FDD28) of the ten-word work-object
 * byte-1 handler run based at 0x801FDD14 and that word is the only image
 * reference to the address, so the byte-1 dispatcher
 * callWorkByte1Step selects it when the work
 * object's byte 1 is 5. The 68 bytes are instruction-for-instruction identical
 * to the sibling byte-2 dispatchers
 * callWorkByte2Step (table 0x801FDD3C),
 * callWorkByte2Step3 (table 0x801FDD50),
 * callWorkByte2Step4 (table 0x801FDD70) and
 * callWorkByte2Step5 (table 0x801FDD8C),
 * differing only in instruction 9, the table-address immediate word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte2Step6(void) {
  D_801FDDAC[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
