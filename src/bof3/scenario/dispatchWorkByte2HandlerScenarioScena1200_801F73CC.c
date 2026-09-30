#include "bof3/bof3.h"

typedef void (*ScenarioScena1200WorkSubStateHandler)(void);

/* Code-pointer run based at 0x801FD190, selected by the work object's byte at
 * offset 0x02: entry 0 is 0x801F7410 and entry 1 is 0x801F7518. The address sits
 * inside the overlay's final asm block, which defines the label D_801FD190; the
 * target map does not own it. */
extern ScenarioScena1200WorkSubStateHandler D_801FD190[];

/* @source 0x801F73CC
 * @behavior Overlay state dispatcher for the work object's byte at offset 0x02:
 * it reads the scratchpad work object published by the pointer cell at
 * 0x1F800044, takes that object's unsigned byte at offset 0x02, scales it by
 * four and invokes that entry of the in-image code-pointer run based at
 * 0x801FD190 with no arguments. The first two words of that run are 0x801F7410
 * and 0x801F7518, so the byte is the state index of the handler window this
 * dispatcher reads. It reads no other state and writes none; the pointer cell is
 * loaded before the 0x18-byte frame is created and that frame only keeps $ra
 * across the indirect call. The table address is built as `lui $at, %hi(table)`
 * / `addu $at, $at, index * 4` / `lw $v0, %lo(table)($at)`. The 68 bytes are
 * instruction-for-instruction identical to the byte-2 dispatchers at 0x801F71B8
 * (this overlay, table 0x801FD184, lifted as
 * dispatchWorkByte2HandlerScenarioScena1200_801F71B8, which records the same
 * identity for 0x801F6E10 and 0x801F6F48 of scena07/00 and 0x801F796C of
 * scena08/00), differing only in that table-address word. The only in-image
 * reference to 0x801F73CC is the pointer word at 0x801FD17C, entry 1 of the
 * byte-1 run at 0x801FD178 that
 * dispatchWorkByte1HandlerScenarioScena1200_801F7174 selects, so this is the
 * frame handler for byte 1 == 1. The name records the proven role and the
 * address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerScenarioScena1200_801F73CC(void) {
  D_801FD190[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
