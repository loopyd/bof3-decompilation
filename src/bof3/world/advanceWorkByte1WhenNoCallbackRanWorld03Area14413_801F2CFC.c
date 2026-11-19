#include "bof3/bof3.h"

/* @source 0x801F34FC @kind function */
extern int func_801F34FC(void);

/* @source 0x801F2CFC
 * @behavior Overlay step handler, entry 2 of the per-frame handler pointer run
 *           at 0x801F3E68 that this overlay's dispatcher 0x801F2C04 indexes with
 *           byte +0x01 of the work record: runs the record-walk helper
 *           0x801F34FC, which visits the work records of the shared table at
 *           0x800E4800 and leaves 1 in its return register once a record's own
 *           handler has run and 0 when none did. Only when the low byte of that
 *           result is zero does this body advance byte +0x01 of the record
 *           published through the scratchpad pointer cell 0x1F800044, which is
 *           the byte that same table is indexed with, so the step is handed to
 *           the next handler of the run. Takes no arguments and returns
 *           nothing; it reads no other state and writes nothing beyond that one
 *           byte. The 0x18-byte frame only keeps $ra at 0x10 across the call and
 *           the increment is reached on the zero path alone.
 *
 *           Representation notes: the scratchpad pointer cell is spelled with
 *           the SPAD_PTR_SLOT(u8, 0x44) macro, the form the target's exact
 *           single-access handlers use, and its five-word increment sequence
 *           (lui 0x1F80, lw 0x44, lbu 0x1, addiu 1, sb) is the one the original
 *           issues at 0x801F2D18..0x801F2D30. The zero test on the callee's low
 *           byte is the caller-side width: 0x801F34FC returns $s4 without a byte
 *           normalisation, so the mask belongs to this body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte1WhenNoCallbackRanWorld03Area14413_801F2CFC(void) {
  if ((func_801F34FC() & 0xFF) == 0) {
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
  }
}
