#include "bof3/bof3.h"

typedef void (*Boss03816StepHandler)(u8 index);

/* This payload's data row at 0x800C1C10, the 0x8C bytes the Splat `textbin`
 * row places at the end of the code segment: its first 0x14 bytes are the byte
 * and flag run this publisher installs, and the pointer runs it also holds
 * start at 0x800C1C24, at 0x800C1C54 and at 0x800C1C60. 0x800C1C1C is that
 * run's own +0xC word, inside it and not a row boundary, so it is referenced as
 * its own data symbol rather than as a row start. */
extern u8 D_800C1C10; /* @source 0x800C1C10 @kind data */
extern u8 D_800C1C1C; /* @source 0x800C1C1C @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed step dispatcher at 0x800C19E4: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C1C90
 * run, the 0x80-byte offset inside the same 0x800C1C10 row. */
void func_800C19E4(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup -- this payload calls it at 0x800C19CC, and 0x800C1C68 and
 * 0x800C1D40 of BIN/BOSS/BOSS008.EMI and BIN/BOSS/BOSS034.EMI do the same; it
 * takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1980
 * @behavior Step setup of this overlay's work state machine, entry 0 of the
 * handler run based at 0x800C1C60: publishes three words into the current enemy
 * work record loaded from 0x801EB4E8 -- the data row pointer 0x800C1C10 into
 * +0xEC, the step dispatcher func_800C19E4 into +0xE4 and the in-row pointer
 * 0x800C1C1C into +0xE8 -- then raises bit 0x8 of the record's +0x104 flag
 * word, stores 2 into the state byte at +1 of the work record the main program
 * publishes at the scratchpad pointer cell 0x1F800044 and calls func_8014D978.
 * No jal in the payload names 0x800C1980: its only in-image reference is the
 * word 0x800C1C60 of that same data row, and func_800C193C dispatches on that
 * very state byte, so writing 2 hands the next step to the run's entry 2. The
 * role of the 0x8 bit in the +0x104 flag word is not established here, and
 * nothing in the payload reads the +0x104 word back. The body equals the first
 * 0x54 bytes of this payload's own func_800C1848 apart from the installed
 * dispatcher's immediate, and it differs from the already-exact publishers
 * publishWorkStepHandlerAndDataThenSetStateTo2BossBoss03016_800C1848 and
 * publishWorkStepHandlerAndDataThenSetStateTo2BossBoss00716_800C1864 only by
 * that +0x104 read-modify-write; both prologue and return path place the
 * boundary.
 * @status partial
 * @match 92.00
 * @residual 23/25 instructions, 100 bytes, first difference at the +0x104 flag
 * read-modify-write: this body emits ori then sw of the flag word before the
 * lui/lw pair that loads the 0x1F800044 pointer cell, and the original has the
 * cell load first and the ori/sw second. Moving the cell read into a pointer
 * local before the flag store puts the load in the original's slot but takes
 * the $a0 the original keeps for the work record, so the three record stores
 * and the state byte then run through $v1 and the cell pointer through $a0
 * (13/25); reading the cell before the +0xE8 store instead hoists the load four
 * slots early into $a1 (22/25), and reading it before the +0xEC store or at the
 * top of the body measures 22/25 at earlier positions. Those are the only
 * positions the store-aliasing rules allow for that cell read, so the residual
 * is the register-allocation order between the two pointers, not another
 * statement shape.
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03816_800C1980(
    void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1C10;
  FIELD_REF(Boss03816StepHandler, work, 0xE4u) = func_800C19E4;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1C1C;
  FIELD_REF(u32, work, 0x104u) |= 8u;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
