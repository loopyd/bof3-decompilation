#include "bof3/bof3.h"

typedef void (*Boss00716StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C1D78 data row, the 160 bytes the Splat `textbin` row places at the end
 * of the segment. 0x800C1D78 is that row's own label; 0x800C1D90 is its +0x18
 * word, the flag-byte run inside the row and not a row boundary, so it is
 * referenced as its own data symbol rather than as a row start. */
extern u8 D_800C1D78; /* @source 0x800C1D78 @kind data */
extern u8 D_800C1D90; /* @source 0x800C1D90 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed step dispatcher at 0x800C18BC: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C1DD0
 * run, the 0x58-byte offset inside the same 0x800C1D78 row, whose entries 0
 * through 2 are the noop handlers at 0x800C18F4, 0x800C18FC and 0x800C1904 and
 * whose entry 3 is the dispatcher at 0x800C1950. */
void func_800C18BC(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1864
 * @behavior Step-0 setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1D78 into +0xEC, the byte dispatcher func_800C18BC into
 * +0xE4 and the flag-byte pointer 0x800C1D90 (0x800C1D78 + 0x18, inside that
 * same row) into +0xE8 -- then stores 2 into the state byte at +1 of the work
 * record the main program publishes at the scratchpad pointer cell 0x1F800044
 * and calls func_8014D978. That state byte is the value func_800C1804 indexes
 * its 0x800C1DA0 table by (row 0x800C1D78 + 0x28), and the word 0x800C1864 is
 * entry 0 of that table, so writing 2 leaves this entry for entry 2; no jal
 * names 0x800C1864 and the sole in-image reference is that table word, so the
 * overlay entry calls it. The body is instruction-for-instruction identical to
 * the already-exact sibling
 * publishWorkStepHandlerAndDataThenSetStateTo2BossBoss03016_800C1848, differing
 * only in the %hi/%lo immediates of what it installs. Both prologue and return
 * path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetStateTo2BossBoss00716_800C1864(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1D78;
  FIELD_REF(Boss00716StepHandler, work, 0xE4u) = func_800C18BC;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1D90;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
