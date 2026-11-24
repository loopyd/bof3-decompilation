#include "bof3/bof3.h"

/* Record pointer cell at 0x801F17B4 and handler table at 0x801F01B0.
 *
 * The overlay entry at 0x801EEC28 walks the 0x80-entry, 0x2C-byte record array
 * based at 0x801F01B4 and, for each record whose byte 0 has bit 0 set, publishes
 * 0x801F01B4 + index * 0x2C into 0x801F17B4 with the `sw` at 0x801EECE8 and
 * calls this dispatcher with `jal` at 0x801EECF8, so the cell holds a live
 * record whenever the index byte is read. The table at 0x801F01B0 ships the
 * single word 0x801EF380, the next function of this overlay. Both addresses sit
 * in this overlay's trailing read-only block, which no map row names, so the
 * references bind through the target's generated undefined-symbol table.
 */
extern u8 *D_801F17B4; /* @source 0x801F17B4 @kind data */
extern void (*D_801F01B0[])(void); /* @source 0x801F01B0 @kind data */

/* @source 0x801EF33C
 * @behavior Dispatches one step of the current bmagic work record. The record
 * pointer is read from the cell at 0x801F17B4 before the frame is created, the
 * record's unsigned byte at offset 1 is scaled by four and selects an entry of
 * the code-pointer table based at 0x801F01B0, and the selected handler is
 * invoked with no arguments. The original bounds neither the byte nor the
 * entry, so a byte past the shipped single-word table reads the record array
 * that starts four bytes after the table base as a handler address. The
 * dispatcher reads no other state and writes none; the 0x18-byte frame only
 * keeps $ra across the indirect call, and the load delays after the index byte
 * and after the table load stay nops. The 68-byte span is byte-identical to
 * emi/bmagic/magic070/03 0x801EF33C and emi/bmagic/magic072/03 0x801EF33C,
 * proven by comparing the payload spans at their own runtime-address offsets.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callDispatchRecordHandler(void) {
  D_801F01B0[D_801F17B4[1]]();
}
