#include "bof3/bof3.h"

typedef void (*ScenarioScena1400RecordCallback)(u8 *record, u32 arg1);

/* Overlay-local record-callback table at 0x801FC468, indexed by the record's
 * unsigned byte field at offset 0x7A: its twenty-two code-pointer words run
 * from func_801FB3BC (0x801FB3BC, word 0) through func_801FC328 (0x801FC328,
 * word 21), word 5 being
 * requestPrimaryState5Substate5ScenarioScena1400_801FB574 (0x801FB574) and
 * word 8 being func_801FBD2C (0x801FBD2C). It begins immediately after the
 * seven data words at 0x801FC44C and ends immediately before the zero word at
 * 0x801FC4C0.
 * @source 0x801FC468 @kind table
 */
extern ScenarioScena1400RecordCallback D_801FC468[];

extern u32 D_8014686C;

/* @source 0x801FB37C
 * @behavior Overlay-local record-callback dispatcher: it reads the record
 * passed in $a0, takes its unsigned byte field at offset 0x7A with
 * `lbu 0x7A($a0)`, scales it by four and selects a callback word of the
 * overlay-local table at 0x801FC468, then invokes that callback with `jalr`,
 * passing the record pointer unchanged as the first argument and the shared
 * word D_8014686C (0x8014686C, read with `lw` into $a1) as the second. The
 * twenty-two table words run from func_801FB3BC (0x801FB3BC, word 0) through
 * func_801FC328 (0x801FC328, word 21). It reads no other state and writes
 * none; the 0x18-byte frame only keeps $ra across the indirect call, and the
 * word load of D_8014686C is scheduled between the record byte load and the
 * index shift, so the original bytes read `lbu 0x7A($a0)`, `lui`/`lw`
 * D_8014686C, then `sll` and the `lui $at`/`addu` table address. No in-image
 * `jal` targets the address; its only image word is 0x801FC408, entry 1 of the
 * five-word pointer run at 0x801FC404
 * (dispatchScenarioStateHandlerScenarioScena1400_801F6FD8 / 0x801FB37C /
 * func_801FB648 / func_801FB84C / 0), which no code in this target reaches
 * directly. Its 64 bytes are instruction-for-instruction identical to the
 * exact record-callback dispatchers
 * dispatchRecordCallbackByByte7AScenarioScena0700_801FD3BC (0x801FD3BC) and
 * dispatchRecordCallbackByByte7AScenarioScena1200_801FC574 (0x801FC574),
 * differing only in the table symbol immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackByByte7AScenarioScena1400_801FB37C(u8 *record) {
  D_801FC468[record[0x7A]](record, D_8014686C);
}
