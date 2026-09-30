#include "bof3/bof3.h"

typedef void (*ScenarioScena0900WorkStateHandler)(void);

/* Overlay-local two-word work-object handler table at 0x801FE384, selected by
 * the scratchpad work object's unsigned byte 1: entry 0 is func_801F73A0
 * (0x801F73A0) and entry 1 is
 * resetWorkStateWhenNoRecordActiveScenarioScena0900_801F73F0 (0x801F73F0, the
 * reset handler this target already lifts); the first non-pointer word,
 * 0x0003FFFE, sits immediately after them at 0x801FE38C. Both words are
 * overlay handler addresses reached only through this dispatcher, and the
 * image word at 0x801FE384 is this table's entry 0, not a call site.
 * @source 0x801FE384 @kind table
 */
extern ScenarioScena0900WorkStateHandler func_801FE384[];

/* @source 0x801F735C
 * @behavior Dispatches this overlay's per-frame work-object handler from the
 * scratchpad: it reads the work-object pointer cell at 0x1F800044 (`lui
 * $v0,0x1F80` then `lw $v0,0x44($v0)`), takes that object's unsigned byte at
 * offset 1, scales it by four and invokes that entry of the overlay-local
 * two-word handler table at 0x801FE384 with no arguments - entry 0 =
 * func_801F73A0 (0x801F73A0) and entry 1 =
 * resetWorkStateWhenNoRecordActiveScenarioScena0900_801F73F0 (0x801F73F0), as
 * this target's already-lifted sibling records. It reads no other state and
 * writes none; the 0x18-byte frame only keeps $ra across the indirect call, and
 * the pointer-cell load precedes frame creation, so the original bytes read
 * `lui`/`lw` 0x1F800044, `addiu $sp,-0x18`, `sw $ra,0x10($sp)`, then `lbu
 * 0x1($v0)`, `sll`, `lui $at`/`addu` and the indexed `lw`. No in-image `jal`
 * and no image data word in this target's payload addresses 0x801F735C, so its
 * caller is outside the overlay. Its 0x44 bytes are instruction-for-instruction
 * identical to the exact sibling dispatchWorkByte1HandlerScenarioScena0100_801F73B8
 * (0x801F73B8), which differs only in the table-entry load immediate
 * (0x801FE26C there), and it shares that shape with the scena07/08/12/15
 * work-byte-1 dispatchers; the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerScenarioScena0900_801F735C(void) {
  func_801FE384[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
