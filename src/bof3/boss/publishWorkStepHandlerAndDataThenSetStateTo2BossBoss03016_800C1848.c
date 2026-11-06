#include "bof3/bof3.h"

typedef void (*Boss03016StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C1AB4 data row, the 160 bytes the Splat `textbin` row places at the end
 * of the segment. 0x800C1AB4 is that row's own label; 0x800C1ACC is its +0x18
 * word, inside the row and not a row boundary, so it is referenced as its own
 * data symbol rather than as a row start. */
extern u8 D_800C1AB4; /* @source 0x800C1AB4 @kind data */
extern u8 D_800C1ACC; /* @source 0x800C1ACC @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's step dispatcher at 0x800C18A0: it masks its argument with 0xFF,
 * scales it by four and calls the entry of the 0x800C1B0C run, whose entries 0
 * through 2 are the noop handlers at 0x800C18D8, 0x800C18E0 and 0x800C18E8 and
 * whose entry 3 is the sibling publisher at 0x800C1934. */
void func_800C18A0(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step-0 setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1AB4 into +0xEC, the step dispatcher func_800C18A0 into
 * +0xE4 and the data pointer 0x800C1ACC (0x800C1AB4 + 0x18, inside that same
 * row) into +0xE8 -- then stores 2 into the state byte at +1 of the work record
 * the main program publishes at the scratchpad pointer cell 0x1F800044 and
 * calls func_8014D978. The state byte is the value func_800C1804 and
 * func_800C18F0 index their 0x800C1ADC and 0x800C1B18 tables by, so writing 2
 * selects the next handler instead of re-entering this one: the word
 * 0x800C1848 at 0x800C1ADC is entry 0 of that state table and this function's
 * only in-image reference, so no jal reaches it and the overlay entry does. The
 * three stored addresses are the only difference from the sibling publisher at
 * 0x800C1934, entry 3 of the 0x800C1B0C step run, which installs a different
 * pair of row pointers and the dispatcher 0x800C198C and also leaves the state
 * byte at 2. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetStateTo2BossBoss03016_800C1848(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1AB4;
  FIELD_REF(Boss03016StepHandler, work, 0xE4u) = func_800C18A0;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1ACC;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
