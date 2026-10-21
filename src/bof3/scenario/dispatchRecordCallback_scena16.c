#include "bof3/scenario/scena16_internal.h"

/* @behavior Dispatches one SCENA16 record callback: it reads the record's 0x7A
 * selector byte with an unsigned byte load, scales it by four and loads the
 * handler from the overlay-local recordCallbackTable (0x801F856C), then calls
 * that handler with the record pointer and the shared work-context word
 * D_8014686C. It reads no other memory, writes none and returns nothing of its
 * own; the 0x18-byte frame exists only to keep $ra across the indirect call.
 *
 * Instruction evidence: the 0x7A byte is unsigned (`lbu`), the table index is
 * scaled with `sll 2` and folded into the `lui $at` address, the context word is
 * loaded with a plain `lw`, and the handler is entered with `jalr` plus a delay
 * nop. The callback takes two arguments and no stack arguments.
 * @source 0x801F8358
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordCallback(void* record) {
  Scena16RecordCallback callback;

  do {
    callback = recordCallbackTable[((const u8*)record)[0x7a]];
    callback(record, D_8014686C);
  } while (0);
}
