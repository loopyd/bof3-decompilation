#include "bof3/bof3.h"

/* @source 0x801F34FC @kind function */
extern int func_801F34FC(void);

/* @source 0x801F30CC @kind function */
extern void stateSixTableRecords(void);

/* @source 0x8015DF18 @kind function */
extern void func_8015DF18(u16 cue_id);

/* @source 0x801F2CA4
 * @behavior Overlay step handler, entry 1 of the per-frame handler pointer run
 *           at 0x801F3E68 that this overlay's dispatcher 0x801F2C04 indexes
 *           with byte +0x01 of the work record: runs the record-walk helper
 *           0x801F34FC, which visits the work records of the shared table at
 *           0x800E4800 and leaves 1 in its return register once a record's own
 *           handler has run and 0 when none did. Only when the low byte of that
 *           result is zero does this body run the step-0 bootstrap 0x801F30CC,
 *           which seeds bytes 0x14 through 0x19 of those six records; hand cue
 *           0x203 to the shared EXE-side cue dispatcher 0x8015DF18; and, after
 *           re-materialising the record pointer from the scratchpad cell
 *           0x1F800044, advance byte +0x01 of that record, the byte this run is
 *           indexed with, so the frame passes to entry 2. It takes no arguments,
 *           returns nothing, reads no state of its own beyond the walk result and
 *           that pointer cell, and reserves 0x18 bytes of frame with $ra at 0x10
 *           across the three calls; the walk's zero path alone reaches the other
 *           two calls and the increment.
 *
 *           Caller evidence: no jal in this payload encodes 0x801F2CA4 and its
 *           single pointer word sits at 0x801F3E70, entry 1 of the run above, so
 *           the dispatcher is its only caller and its argument roles stay
 *           structural.
 *
 *           Representation notes: the statement order is the original's, the
 *           two jal instructions at 0x801F2CC0 and 0x801F2CC8 preceding the
 *           increment group at 0x801F2CD0..0x801F2CE8. The zero test is written
 *           on the masked low byte, the caller-side width 0x801F34FC does not
 *           normalise itself. The scratchpad pointer cell is spelled with the
 *           SPAD_PTR_SLOT(u8, 0x44) macro, the form the target's exact
 *           single-access handlers use, and it yields the original's five-word
 *           increment sequence lui 0x1F80, lw 0x44, lbu 0x1, addiu 1, sb. The
 *           0x801F30CC call passes nothing, as the original leaves its argument
 *           register untouched, and the 0x8015DF18 call passes the literal cue
 *           id, which the original materialises with addiu $a0, $zero, 0x203.
 * @status exact
 * @match 100.00
 * @residual none
 */
void statePlayNoRan(void) {
  if ((func_801F34FC() & 0xFF) == 0) {
    stateSixTableRecords();
    func_8015DF18(0x203);
    SPAD_PTR_SLOT(u8, 0x44)[1]++;
  }
}
