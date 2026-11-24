#include "bof3/bof3.h"

typedef void (*ScenarioScena0700WorkSubStateHandler)(void);

/* Overlay-local run of in-image code pointers based at 0x801FDDC8, ten words
 * (0x801FDDC8..0x801FDDEC) ending at the next data label 0x801FDDF0: its first
 * words are 0x801F7D6C, 0x801F7DDC, 0x801F7E68, 0x801F7EF8, 0x801F7FA4,
 * 0x801F805C, 0x801F80F0, incrementWorkHandlerIndex,
 * incrementWorkHandlerIndex2 and
 * callOverlayHelper5 (the word at 0x801FDDEC), and the
 * separate label D_801FDDF0 at 0x801FDDF0 - the work-table callback row the
 * exact countdownAdvanceHandler belongs
 * to - closes it. The address is word 2 of the twelve-word code-pointer run
 * based at 0x801FDDC0 that the exact sibling
 * callWorkByte2Step8 indexes, so this table is
 * that run's suffix from its third word. It sits inside the overlay's final
 * Splat asm block (the code-shaped boundary func_801FDD14), which defines the
 * label; the target map does not own it. */
extern ScenarioScena0700WorkSubStateHandler D_801FDDC8[];

/* @source 0x801F7D28
 * @behavior Overlay handler dispatcher for the scratchpad work object's byte at
 * offset 0x02: it loads the work-object pointer cell at 0x1F800044 (`lui
 * $v0,0x1F80` then `lw $v0,0x44($v0)`) before it creates its frame, takes that
 * object's unsigned byte at offset 0x02 (`lbu $v0,0x2($v0)`), scales it by four
 * (`sll $v0,$v0,2`) and invokes that slot of the overlay-local in-image
 * code-pointer run based at 0x801FDDC8 (`lui $at,%hi` / `addu $at,$at,$v0` /
 * `lw $v0,%lo($at)` / `jalr $v0`) with no arguments and no result consumed, so
 * that byte is the state index of the handler window this dispatcher reads. It
 * reads no state beyond that byte and writes none; its 0x18-byte frame exists
 * only to keep $ra across the indirect call, whose delay slot stays a nop, and
 * the pointer-cell load precedes frame creation. 0x801F7D28 is entry 8 (the
 * word at 0x801FDD34) of the ten-word work-object byte-1 handler run based at
 * 0x801FDD14 - whose own source
 * callWorkByte1Step records this address as
 * the run's ninth word - and that word is the only image reference to the
 * address, so the byte-1 dispatcher selects it when the work object's byte 1 is
 * 8. The 68 bytes are instruction-for-instruction identical to the byte-2
 * dispatchers of the already-exact siblings
 * callWorkByte2Step (table 0x801FDD3C),
 * callWorkByte2Step3 (table 0x801FDD50),
 * callWorkByte2Step4 (table 0x801FDD70),
 * callWorkByte2Step5 (table 0x801FDD8C),
 * callWorkByte2Step6 (table 0x801FDDAC),
 * callWorkByte2Step7 (table 0x801FDDB4),
 * callWorkByte2Step8 (table 0x801FDDC0) and
 * this lane's callWorkByte2Step10 (table
 * 0x801FDDD0), differing only in instruction 9, the table-address immediate
 * word.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callWorkByte2Step9(void) {
  D_801FDDC8[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
