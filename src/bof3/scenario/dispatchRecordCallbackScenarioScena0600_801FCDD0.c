#include "bof3/bof3.h"

typedef void (*ScenarioScena0600RecordCallback)(u8 *record, u32 arg1);

extern u32 D_8014686C;

/* Overlay-local per-record callback table at 0x801FE42C, indexed by the
 * record's unsigned byte field at offset 0x7A: its fourteen code-pointer words
 * run from the inert noopHandlerScenarioScena0600_801FCE10 (0x801FCE10, word
 * 0) through func_801FD234 (0x801FD234, word 13, the last pointer word), and
 * the first non-pointer word, 0x181E3232, sits immediately after them at
 * 0x801FE464. Among the other words are the reviewed
 * setScenarioFlagBit0CScenarioScena0600_801FCF98 (0x801FCF98, word 5) and this
 * target's own record handlers func_801FCE18 (0x801FCE18, word 1) and
 * func_801FCE6C (0x801FCE6C, word 2). Every word is an overlay handler reached
 * only through this dispatcher.
 * @source 0x801FE42C @kind table
 */
extern ScenarioScena0600RecordCallback D_801FE42C[];

/* @source 0x801FCDD0
 * @behavior Overlay-local record-callback dispatcher: it reads the record
 * passed in $a0, takes its unsigned byte field at offset 0x7A, scales it by
 * four and selects a callback word of the overlay-local table at 0x801FE42C,
 * then invokes that callback with `jalr`, passing the record pointer unchanged
 * as the first argument and the shared word D_8014686C (0x8014686C, read with
 * `lw` into $a1) as the second. It reads no other state and writes none; the
 * 0x18-byte frame only keeps $ra across the indirect call and the callback's
 * return value in $v0 is discarded, and the word load of D_8014686C is
 * scheduled between the record byte load and the index shift, so the original
 * bytes read `lbu 0x7A($a0)`, `lui`/`lw` D_8014686C, then `sll` and the
 * `lui $at`/`addu` table address. No in-image `jal` targets the address: its
 * only image word is the pointer at 0x801FE3C8, the second slot of the
 * five-word code-pointer run 0x801FE3C4..0x801FE3D4 (0x801F7298, this address,
 * 0x801FD800, 0x801FD294, 0x801FDD14) that immediately precedes the three-word
 * outer-state table at 0x801FE3D8 used by
 * dispatchScenarioStateHandlerScenarioScena0600_801F7298. Its 64 bytes are
 * instruction-for-instruction identical to the exact sibling
 * dispatchRecordCallbackScenarioScena0900_801FC5E4 (0x801FC5E4 of
 * emi/scenario/scena09/00), which differs only in the table-entry load
 * immediate (0x801FE434 there), and the same shape and name are used by the
 * other scenario overlays' record-callback dispatchers, so the address anchor
 * keeps this name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackScenarioScena0600_801FCDD0(u8 *record) {
  D_801FE42C[record[0x7A]](record, D_8014686C);
}
