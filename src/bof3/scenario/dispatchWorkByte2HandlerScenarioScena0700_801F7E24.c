#include "bof3/bof3.h"

typedef void (*ScenarioScena0700WorkSubStateHandler)(void);

/* Overlay-local run of in-image code pointers based at 0x801FDDD0, eight words
 * (0x801FDDD0..0x801FDDEC) ending at the next data label 0x801FDDF0: its words
 * are 0x801F7E68, 0x801F7EF8, 0x801F7FA4, 0x801F805C, 0x801F80F0,
 * incrementWorkByte2ScenarioScena0700_801F8174,
 * incrementWorkByte2ScenarioScena0700_801F8194 and
 * invokeHelperScenarioScena0700_801F81B4 (the word at 0x801FDDEC), and the
 * separate label D_801FDDF0 at 0x801FDDF0 - the work-table callback row the
 * exact countdownReload32ThenAdvanceHandlerScenarioScena0700_801F8730 belongs
 * to - closes it. The address is word 4 of the twelve-word code-pointer run
 * based at 0x801FDDC0 that the exact sibling
 * dispatchWorkByte2HandlerScenarioScena0700_801F7B9C indexes, so this table is
 * that run's suffix from its fifth word. It sits inside the overlay's final
 * Splat asm block (the code-shaped boundary func_801FDD14), which defines the
 * label; the target map does not own it. */
extern ScenarioScena0700WorkSubStateHandler D_801FDDD0[];

/* @source 0x801F7E24
 * @behavior Overlay handler dispatcher for the scratchpad work object's byte at
 * offset 0x02: it loads the work-object pointer cell at 0x1F800044 (`lui
 * $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its frame, takes that
 * object's unsigned byte at offset 0x02 (`lbu $v0,0x2($v0)`), scales it by four
 * (`sll $v0,$v0,2`) and invokes that slot of the overlay-local in-image
 * code-pointer run based at 0x801FDDD0 (`lui $at,%hi` / `addu $at,$at,$v0` /
 * `lw $v0,%lo($at)` / `jalr $v0`) with no arguments and no result consumed, so
 * that byte is the state index of the handler window this dispatcher reads. It
 * reads no state beyond that byte and writes none; its 0x18-byte frame exists
 * only to keep $ra across the indirect call, whose delay slot stays a nop, and
 * the pointer-cell load precedes frame creation. 0x801F7E24 is entry 9 (the
 * word at 0x801FDD38) of the ten-word work-object byte-1 handler run based at
 * 0x801FDD14 - whose own source
 * dispatchWorkByte1HandlerScenarioScena0700_801F6DCC records this address as
 * the run's tenth and last word - and that word is the only image reference to
 * the address, so the byte-1 dispatcher selects it when the work object's byte
 * 1 is 9. The 68 bytes are instruction-for-instruction identical to the byte-2
 * dispatchers of the already-exact siblings
 * dispatchWorkByte2HandlerScenarioScena0700_801F6E10 (table 0x801FDD3C),
 * dispatchWorkByte2HandlerScenarioScena0700_801F700C (table 0x801FDD50),
 * dispatchWorkByte2HandlerScenarioScena0700_801F73C4 (table 0x801FDD70),
 * dispatchWorkByte2HandlerScenarioScena0700_801F75F4 (table 0x801FDD8C),
 * dispatchWorkByte2HandlerScenarioScena0700_801F786C (table 0x801FDDAC),
 * dispatchWorkByte2HandlerScenarioScena0700_801F79EC (table 0x801FDDB4),
 * dispatchWorkByte2HandlerScenarioScena0700_801F7B9C (table 0x801FDDC0) and
 * this lane's dispatchWorkByte2HandlerScenarioScena0700_801F7D28 (table
 * 0x801FDDC8), differing only in instruction 9, the table-address immediate
 * word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerScenarioScena0700_801F7E24(void) {
  D_801FDDD0[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
