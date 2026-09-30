#include "bof3/bof3.h"

typedef void (*BmagicMagic06403Handler)(void);

void func_801F04A0(void);
void func_801F05A4(void);

/* Pointer cell at 0x801F28B4 holding the bmagic work record that is being
 * stepped: 0x801F009C stores &D_801F13B4[index * 0x1C] into the cell and calls
 * this dispatcher immediately afterwards, and both handlers of the local table
 * read the same cell back. The cell sits in this overlay's trailing data block,
 * which no map row owns, so the reference is bound by the target's generated
 * undefined-symbol table. */
extern u8 *D_801F28B4; /* @source 0x801F28B4 @kind data */

/* @source 0x801F0448
 * @behavior Selects and runs one step of the current bmagic work record. The
 * record pointer is read from the cell at 0x801F28B4, and the record's unsigned
 * byte at offset 1 selects one of the two handler addresses seeded into this
 * function's own 0x20-byte frame (entry 0 = 0x801F04A0, which seeds the record
 * motion fields and advances that byte; entry 1 = 0x801F05A4, which integrates
 * the record position by its velocity), and the selected entry is invoked with
 * no arguments. It reads no state other than that byte and writes none; the
 * frame only keeps $ra across the indirect call, whose return address load
 * follows a nop delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkRecordByte1HandlerBmagicMagic06403_801F0448(void) {
  BmagicMagic06403Handler handlers[2] = { func_801F04A0, func_801F05A4 };

  handlers[D_801F28B4[1]]();
}
