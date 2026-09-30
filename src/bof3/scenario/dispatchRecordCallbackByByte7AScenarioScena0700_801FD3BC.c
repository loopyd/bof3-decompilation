#include "bof3/bof3.h"

typedef void (*ScenarioScena0700RecordCallback)(u8* record, u32 arg1);

extern ScenarioScena0700RecordCallback D_801FDF8C[];
extern u32 D_8014686C;

/* @source 0x801FD3BC
 * @behavior Dispatches this overlay's per-record callback selected by the
 * record's unsigned byte field at offset 0x7A through the overlay-local
 * callback-pointer table at 0x801FDF8C (five entries: 0x801FD3FC, 0x801FD45C,
 * 0x801FD4C4, 0x801FD524 and 0x801FD5A0, immediately before the two data words
 * and the five-slot pointer run at 0x801FDFA8), passing the record pointer
 * unchanged as the first argument and the word held in the shared main-RAM cell
 * D_8014686C as the second. It reads no state other than the record byte and
 * that shared cell and writes none; it takes the record as its only argument and
 * returns nothing, and the 0x18-byte frame saves $ra because the target callback
 * is an ordinary indirect call rather than a sibling tail call (the shared cell
 * is read before the index is scaled by four). The 64 bytes are
 * instruction-for-instruction identical to the exact record-callback dispatchers
 * dispatchRecordCallbackScenarioScena1500_801FDD80 (0x801FDD80),
 * dispatchRecordCallbackScenarioScena1100_801FA5B8 (0x801FA5B8) and
 * dispatchRecordCallbackScenarioScena0800_801FE018 (0x801FE018), and to the
 * exact scena18 dispatcher dispatchRecordCallback_scena18 (0x801F6CAC); each
 * differs only in the symbol immediate of the table-entry load. Address
 * 0x801FD3BC is itself entry 0 of the four-slot pointer run at 0x801FDE00 that
 * continues with 0x801FD67C, 0x801FD960 and 0x801FDA98.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallbackByByte7AScenarioScena0700_801FD3BC(u8* record) {
  D_801FDF8C[record[0x7A]](record, D_8014686C);
}
