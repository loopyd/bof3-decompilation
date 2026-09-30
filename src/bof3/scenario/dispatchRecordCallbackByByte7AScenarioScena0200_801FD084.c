#include "bof3/bof3.h"

typedef void (*ScenarioScena0200RecordCallback)(u8* record, u8* work);

/* Overlay-local record-callback table, indexed by a record's unsigned byte
 * field at offset 0x7A: its thirty in-image code-pointer words run from the
 * inert noopHandlerScenarioScena0200_801FD0C4 (0x801FD0C4, entry 0) through
 * func_801FE1C0 (0x801FE1C0, entry 29), and entry 16 holds
 * requestPrimaryState10Substate0FAndQueueCue106ScenarioScena0200_801FD450
 * (0x801FD450). The thirty words are the last 0x78 bytes of the overlay
 * payload, immediately after the data blob at 0x801FE26C.
 * @source 0x801FE304 @kind table
 */
extern ScenarioScena0200RecordCallback D_801FE304[];

extern u8 D_80144E98;

/* @source 0x801FD084
 * @behavior Overlay-local record-callback dispatcher: it takes the record
 * pointer passed in $a0, reads its unsigned byte field at offset 0x7A with
 * `lbu`, scales that index by four and selects a callback word of the
 * overlay-local table at 0x801FE304, then invokes the selected callback with
 * `jalr`, passing the record pointer unchanged as the first argument and the
 * address of the shared main-RAM byte D_80144E98 (0x80144E98, materialised
 * with `lui`/`addiu` into $a1) as the second. It reads no other state and
 * writes none; the 0x18-byte frame only keeps $ra across the indirect call,
 * and the original bytes carry a `nop` in the $v0 load-delay slot between
 * `lbu 0x7A($a0)` and the index `sll`, so the instruction shape is the
 * `lbu`, `nop`, `sll`, `lui $at`/`addu $at,$at,$v0`/`lw` sequence. No in-image
 * `jal` targets the address and no copy of its pointer word was found besides
 * the pointer at 0x801FE278, the fourth word of the data blob at 0x801FE26C
 * (directly after the pointer to
 * dispatchProgressHandlerScenarioScena0200_801F719C at 0x801F719C), which is
 * how the overlay reaches it. Its 0x40 bytes are instruction-for-instruction
 * identical to the exact sibling
 * dispatchRecordCallbackByByte7AScenarioScena0100_801FD444 (0x801FD444,
 * scena01/00), which differs only in the table address (0x801FE308) and the
 * shared byte it passes (&D_80144E90); the same shape and name are used by
 * scena00's dispatchRecordCallbackByByte7A (0x801FC7D0), scena07/00's
 * dispatchRecordCallbackByByte7AScenarioScena0700_801FD3BC (0x801FD3BC),
 * scena12/00's dispatchRecordCallbackByByte7AScenarioScena1200_801FC574
 * (0x801FC574) and scena14/00's
 * dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C (0x801FB37C), each
 * against its own table and shared word. The address anchor keeps this member
 * target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackByByte7AScenarioScena0200_801FD084(u8* record) {
  D_801FE304[record[0x7A]](record, &D_80144E98);
}
