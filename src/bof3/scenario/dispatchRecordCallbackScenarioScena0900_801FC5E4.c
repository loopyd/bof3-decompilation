#include "bof3/bof3.h"

typedef void (*ScenarioScena0900RecordCallback)(u8 *record, u32 arg1);

extern u32 D_8014686C;

/* Overlay-local per-record callback table at 0x801FE434, indexed by the
 * record's unsigned byte field at offset 0x7A: its sixteen code-pointer words
 * run from the inert noopHandlerScenarioScena0900_801FC624 (0x801FC624, word
 * 0) through requestPrimaryState0FSubstate28ScenarioScena0900_801FC8E4
 * (0x801FC8E4, word 15), and the first non-pointer word, 0x072E5575, sits
 * immediately after them at 0x801FE474. Every word is an overlay handler
 * address reached only through this dispatcher.
 * @source 0x801FE434 @kind table
 */
extern ScenarioScena0900RecordCallback D_801FE434[];

/* @source 0x801FC5E4
 * @behavior Overlay-local record-callback dispatcher: it reads the record
 * passed in $a0, takes its unsigned byte field at offset 0x7A, scales it by
 * four and selects a callback word of the overlay-local table at 0x801FE434,
 * then invokes that callback with `jalr`, passing the record pointer unchanged
 * as the first argument and the shared word D_8014686C (0x8014686C, read with
 * `lw` into $a1) as the second. The sixteen table words are the handlers
 * 0x801FC624 (word 0, the inert noopHandlerScenarioScena0900_801FC624) through
 * 0x801FC8E4 (word 15), so the dispatch reaches the whole run of the overlay's
 * record callbacks, among them this target's
 * clearProgressThenRequestPrimaryState5Substate5ScenarioScena0900_801FC668
 * (0x801FC668, word 2). It reads no other state and writes none; the 0x18-byte
 * frame only keeps $ra across the indirect call, and the word load of
 * D_8014686C is scheduled between the record byte load and the index shift, so
 * the original bytes read `lbu 0x7A($a0)`, `lui`/`lw` D_8014686C, then `sll`
 * and the `lui $at`/`addu` table address. No in-image `jal` targets the
 * address; its only image word is the pointer word at 0x801FE3D0, the second
 * slot of the five-word pointer run 0x801FE3CC..0x801FE3DC that immediately
 * precedes the twenty-word state-handler table at 0x801FE3E0. Its 64 bytes are
 * instruction-for-instruction identical to the exact sibling
 * dispatchRecordCallbackByByte7A (0x801FC7D0) of the scena00 overlay, which
 * differs only in the table-entry load immediate (0x801FCA84 there), and the
 * same shape and name are used by the other scenario overlays' record-callback
 * dispatchers; the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackScenarioScena0900_801FC5E4(u8 *record) {
  D_801FE434[record[0x7A]](record, D_8014686C);
}
