#include "bof3/bof3.h"

/* @source 0x8014832E @kind unknown */
extern volatile u8 D_8014832E;

/* @source 0x801F4960 @kind unknown */
extern u8 D_801F4960;

/* @source 0x801F2E18 @kind function */
extern void func_801F2E18(void);

/* @source 0x1F800044 @kind data */
extern u8* D_1F800044;

/* @source 0x801F2C48
 * @behavior Overlay step-0 handler, entry 0 of the per-frame handler table at
 *           0x801F3E68 that this overlay's dispatcher 0x801F2C04 indexes with
 *           byte +0x01 of the work record: latches the shared main-RAM byte
 *           0x8014832E into byte +0x06 of the work record published through the
 *           scratchpad pointer cell 0x1F800044, stores 0x1B into that shared
 *           byte, then calls 0x801F2E18, which seeds the six 0x1C-byte-stride
 *           records of the main-RAM work table at 0x800E4800 and calls
 *           0x801F376C once per record. The call clobbers the pointer register,
 *           so the record pointer is materialised a second time from the same
 *           scratchpad cell; on that reloaded pointer the body zeroes the byte
 *           at 0x801F4960 and advances byte +0x01 by one, which is the dispatch
 *           byte this handler's own table entry was selected with. The cell
 *           0x801F4960 is the cue index that 0x801F34FC cycles through 0..2 while
 *           indexing the local halfword cue table at 0x801F3E60, so zeroing it
 *           restarts that cycle for the step the incremented dispatch byte
 *           selects. Takes no arguments, returns nothing and reserves 0x18 bytes
 *           of frame with $ra at 0x10; the tail store to byte +0x01 is issued
 *           after that $ra reload. The handler table word at 0x801F3E68 names
 *           this span and no `jal` in the payload reaches it.
 *
 *           Representation notes: the scratchpad pointer cell is spelled as the
 *           symbolic extern D_1F800044 rather than through the
 *           SPAD_PTR_SLOT(u8, 0x44) macro this target's single-access handlers
 *           use. With the macro the two accesses straddling the call let the
 *           compiler keep the constant 0x1F800044 in a callee-saved register,
 *           emitting lui+ori into $s0, saving and restoring it and loading the
 *           cell at offset 0; that candidate measured 15 of 23 instructions and
 *           92->108 bytes. The symbolic spelling lets the compiler rebuild the
 *           high half per access and address the cell through the 0x44
 *           displacement, which is the form the original shows, and measured 23
 *           of 23 instructions at 92->92 bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void latchShared832EIntoWorkByte6SetShared832ETo1BBuildWorkTableResetCueIndexAndAdvanceWorkByte1World03Area14413_801F2C48(
    void) {
  u8* work;

  work = D_1F800044;
  work[6] = D_8014832E;
  D_8014832E = 0x1B;
  func_801F2E18();
  work = D_1F800044;
  D_801F4960 = 0;
  work[1]++;
}
