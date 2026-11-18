#include "bof3/bof3.h"

typedef void (*ScenarioScena0500RecordCallback)(u8 *record, u32 arg1);

extern u32 D_8014686C;

/* Overlay-local per-record callback table at 0x801FD644, indexed by the
 * record's unsigned byte field at offset 0x7A: its nine code-pointer words run
 * from the inert noopHandlerScenarioScena0500_801FCA98 (0x801FCA98, word 0)
 * through func_801FCD0C (0x801FCD0C, word 8, the last pointer word), and the
 * first non-pointer word, 0x07108434, sits immediately after them at
 * 0x801FD668. Every word is an overlay handler address reached only through
 * this dispatcher. These nine words are the last nine of the thirty-two-word
 * overlay handler run that this target's primary state dispatcher declares from
 * 0x801FD5E8, so this table is bound at the interior word the scaled index adds
 * to and that longer run keeps its single declaration.
 * @source 0x801FD644 @kind table
 */
extern ScenarioScena0500RecordCallback D_801FD644[];

/* @source 0x801FCA58
 * @behavior Overlay-local record-callback dispatcher: it reads the record
 * passed in $a0, takes its unsigned byte field at offset 0x7A, scales it by
 * four and selects a callback word of the overlay-local table at 0x801FD644,
 * then invokes that callback with `jalr`, passing the record pointer unchanged
 * as the first argument and the shared word D_8014686C (0x8014686C, read with
 * `lw` into $a1) as the second. The nine table words are the handlers
 * 0x801FCA98 (word 0, the inert noopHandlerScenarioScena0500_801FCA98) through
 * 0x801FCD0C (word 8), so the dispatch reaches that run of the overlay's record
 * callbacks. It reads no other state and writes none; the 0x18-byte frame only
 * keeps $ra across the indirect call, and the word load of D_8014686C is
 * scheduled between the record byte load and the index shift, so the original
 * bytes read `lbu 0x7A($a0)`, `lui`/`lw` D_8014686C, then `sll` and the
 * `lui $at`/`addu` table address. No in-image `jal` targets the address; its
 * only image word is the pointer at 0x801FD5CC, the second slot of the
 * five-word code-pointer run 0x801FD5C8..0x801FD5D8 (0x801F7A78, this address,
 * 0x801FCD68, 0x801FCD60, 0x801FD110) that immediately precedes the three-word
 * outer-state table at 0x801FD5DC. Its 64 bytes were identical to the exact
 * sibling dispatchers of scena00 (0x801FC7D0), scena06 (0x801FCDD0) and scena09
 * (0x801FC5E4) in every one of the sixteen words except the table-entry load
 * immediate at offset 0x20, which differs because each overlay dispatches
 * through its own table; the address anchor keeps this name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackScenarioScena0500_801FCA58(u8 *record) {
  D_801FD644[record[0x7A]](record, D_8014686C);
}
