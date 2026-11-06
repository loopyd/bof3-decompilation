#include "bof3/bof3.h"

/* Record pointer cell at 0x801F1054 and handler table at 0x801F0404.
 *
 * The overlay's frame loop publishes the record under test into 0x801F1054 with
 * `sw &D_801F0454[i * 0x20]` at 0x801EECAC and calls this dispatcher at
 * 0x801EECB8, so the cell always holds a live record when the index is read.
 * The table this function indexes runs from 0x801F0404 for six words
 * (0x801EF384, 0x801EFD7C, 0x801EF40C, 0x801EF5CC, 0x801EF704, 0x801EF880) and
 * is preceded at 0x801F0400 by the work-object phase table another dispatcher of
 * this overlay uses. Both cells sit in the target's trailing data block, which no
 * map row owns, so the references bind through the harness address encoding.
 */
extern u8 *D_801F1054; /* @source 0x801F1054 @kind data */
extern void (*D_801F0404[])(void); /* @source 0x801F0404 @kind data */

/* @source 0x801EF340
 * @behavior Selects and runs one step of the current bmagic work record. The
 * record pointer is read from the cell at 0x801F1054 before the frame is
 * created, the record's unsigned byte at offset 1 is scaled by four and selects
 * an entry of the code-pointer table based at 0x801F0404, and the selected
 * handler is invoked with no arguments. The original bounds neither the byte nor
 * the entry, reads no other state and writes none; the 0x18-byte frame only
 * keeps $ra across the indirect call, and the load delays after the index byte
 * and after the table load stay nops. The 68-byte body is
 * instruction-for-instruction identical to emi/bmagic/magic073/03 0x801EF724,
 * differing only in the two symbol immediates.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkRecordByte1HandlerBmagicMagic06303_801EF340(void) {
  D_801F0404[D_801F1054[1]]();
}
