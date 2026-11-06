#include "bof3/bof3.h"

typedef void (*ScenarioScena0400RecordCallback)(u8* record, u32 arg1);

extern u32 D_8014686C;

/* Overlay-local record-callback table at 0x801FA224, indexed by a record's
 * unsigned byte 0x7A: one code-pointer word, 0x801F9D88
 * (setFlagBit0RecordCallbackScenarioScena0400_801F9D88), then the non-pointer
 * word 0x05068330 at 0x801FA228, so index 0 is its only in-image entry.
 * @source 0x801FA224 @kind table
 */
extern ScenarioScena0400RecordCallback D_801FA224[];

/* @source 0x801F9D48
 * @behavior Dispatches this overlay's per-record callback selected by the
 * record's unsigned byte field at offset 0x7A: that byte is loaded with `lbu`,
 * scaled by four and selects a word of the overlay-local callback-pointer table
 * at 0x801FA224, whose only code word is
 * setFlagBit0RecordCallbackScenarioScena0400_801F9D88 (0x801F9D88). The selected
 * callback is called with the record pointer unchanged as the first argument and
 * the word held in the shared main-RAM scenario flag cell D_8014686C as the
 * second, which is the pair that callback documents receiving. It reads no other
 * state and writes none: the 0x18-byte frame exists only to keep $ra across the
 * indirect call, and the callback's return value is left in $v0. No in-image jal
 * targets this address; its only image word is 0x801FA1F4, word 4 of the
 * eight-word code-pointer run at 0x801FA1E4 (0x801F7AC8, 0x801F7B00, 0x801F7BC0,
 * 0x801F8124, 0x801F9D48, 0x801F9DB4, 0x801F9DAC, 0x801FA010) that sits just
 * ahead of the D_801FA204 progress table. Its 64 bytes are
 * instruction-for-instruction identical to the exact record-callback dispatchers
 * dispatchRecordCallbackScenarioScena0300_801FC46C (0x801FC46C),
 * dispatchRecordCallbackScenarioScena0800_801FE018 (0x801FE018) and
 * dispatchRecordCallbackScenarioScena1500_801FDD80 (0x801FDD80), differing only
 * in the table-entry load immediate.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackScenarioScena0400_801F9D48(u8* record) {
  D_801FA224[record[0x7A]](record, D_8014686C);
}
