#include "bof3/bof3.h"

typedef void (*ScenarioScena1200RecordCallback)(u8 *record, u32 arg1);

/* Overlay-local record-callback table, indexed by the record's unsigned byte
 * field at offset 0x7A: its fifteen code-pointer words run from the inert
 * noopHandlerScenarioScena1200_801FC5B4 (0x801FC5B4) through
 * requestPrimaryState1Substate10ScenarioScena1200_801FC7D4 (0x801FC7D4).
 * @source 0x801FD1EC @kind table
 */
extern ScenarioScena1200RecordCallback D_801FD1EC[];

extern u32 D_8014686C;

/* @source 0x801FC574
 * @behavior Overlay-local record-callback dispatcher: it reads the record
 * passed in $a0, takes its unsigned byte field at offset 0x7A, scales it by
 * four and selects a callback word of the overlay-local table at 0x801FD1EC,
 * then invokes that callback with `jalr`, passing the record pointer unchanged
 * as the first argument and the shared word D_8014686C (0x8014686C, read with
 * `lw` into $a1) as the second. The fifteen table words run from the inert
 * noopHandlerScenarioScena1200_801FC5B4 (0x801FC5B4, entry 0) through
 * requestPrimaryState1Substate10ScenarioScena1200_801FC7D4 (0x801FC7D4). It
 * reads no other state and writes none; the 0x18-byte frame only keeps $ra
 * across the indirect call, and the word load of D_8014686C is scheduled
 * between the record byte load and the index shift, so the original bytes read
 * `lbu 0x7A($a0)`, `lui`/`lw` D_8014686C, then `sll` and the `lui $at`/`addu`
 * table address. No in-image `jal` targets the address; the only in-image
 * reference is the pointer word at 0x801FD1A8, entry 12 of the overlay's
 * per-frame work handler table at 0x801FD178, which func_801F7174 selects with
 * the state byte at offset 0x01 of the scratch work object published at
 * 0x1F800044. The 64 bytes are instruction-for-instruction identical to the
 * exact scena18 twin dispatchRecordCallback_scena18 (0x801F6CAC), which differs
 * only in the table address (0x801F6D7C), and the same shape and name are used
 * by scena00's dispatchRecordCallbackByByte7A (0x801FC7D0) and scena01/00's
 * dispatchRecordCallbackByByte7AScenarioScena0100_801FD444 (0x801FD444); the
 * address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackByByte7AScenarioScena1200_801FC574(u8 *record) {
  D_801FD1EC[record[0x7A]](record, D_8014686C);
}
