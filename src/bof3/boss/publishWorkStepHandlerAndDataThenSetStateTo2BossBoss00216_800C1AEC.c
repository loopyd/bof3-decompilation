#include "bof3/bof3.h"

typedef void (*Boss00216StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C1F9C data row, the 244 bytes the Splat `textbin` row places at the end
 * of the segment. 0x800C1F9C is that row's own label; 0x800C1FAC is its +0x10
 * word, the 0x0600 halfword run inside the row and not a row boundary, so it is
 * referenced as its own data symbol rather than as a row start. */
extern u8 D_800C1F9C; /* @source 0x800C1F9C @kind data */
extern u8 D_800C1FAC; /* @source 0x800C1FAC @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed handler dispatcher at 0x800C1C08: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C2048
 * run, whose entries 0 through 2 are the noop handlers at 0x800C1C40,
 * 0x800C1C48 and 0x800C1C50. That dispatcher is still an `asm` boundary in this
 * target, so it is named here by its map symbol. */
void func_800C1C08(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1AEC
 * @behavior Step setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1F9C into +0xEC, the byte dispatcher func_800C1C08 into
 * +0xE4 and the in-row data pointer 0x800C1FAC (0x800C1F9C + 0x10) into +0xE8
 * -- then stores 2 into the state byte at +1 of the work record the main
 * program publishes at the scratchpad pointer cell 0x1F800044 and calls
 * func_8014D978. The unselected sibling at 0x800C1B88 writes the same state
 * byte with 3 and clears the bytes at +2 and +3, so +1 is the overlay's state
 * selector and 2 is the step this publisher hands to. No jal names 0x800C1AEC;
 * its sole in-image reference is the table word 0x800C2000, entry 3 of the
 * 0x800C1FF4 handler table the dispatcher at 0x800C19C8 indexes, so the main
 * program reaches it through that dispatch. The body is instruction-for-
 * instruction identical to the already-exact siblings at 0x800C1864 of
 * boss007/16, 0x800C1848 of boss030/16 and 0x800C1D1C of boss008/16, differing
 * only in the addresses it installs and calls. Both prologue and return path
 * place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetStateTo2BossBoss00216_800C1AEC(
    void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1F9C;
  FIELD_REF(Boss00216StepHandler, work, 0xE4u) = func_800C1C08;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1FAC;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
