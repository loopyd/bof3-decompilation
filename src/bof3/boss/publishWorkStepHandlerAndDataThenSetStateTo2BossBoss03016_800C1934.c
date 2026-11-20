#include "bof3/bof3.h"

typedef void (*Boss03016StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C1AB4 data row, the 160 bytes the Splat `textbin` row places at the end
 * of the segment. 0x800C1AC0 is that row's +0xC word, the six halfwords
 * 0x0080, 0x0282, 0x0383, 0x0282, 0x0080 and 0x0000, and 0x800C1AD4 is its
 * +0x20 word, the 0x0600 halfword followed by three 0xFFFF sentinels; both sit
 * inside the row and not on a row boundary, so each is referenced as its own
 * data symbol rather than as a row start. */
extern u8 D_800C1AC0; /* @source 0x800C1AC0 @kind data */
extern u8 D_800C1AD4; /* @source 0x800C1AD4 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed step dispatcher at 0x800C198C: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C1B48
 * run, whose entries 0 through 2 are the noop handlers at 0x800C19C4,
 * 0x800C19CC and 0x800C19D4. */
void dispatchIndexedHandlerBossBoss03016_800C198C(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1934
 * @behavior Step setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the in-row
 * data pointer 0x800C1AC0 into +0xEC, the byte dispatcher
 * dispatchIndexedHandlerBossBoss03016_800C198C into +0xE4 and the in-row data
 * pointer 0x800C1AD4 into +0xE8 -- then stores 2 into the state byte at +1 of
 * the work record the main program publishes at the scratchpad pointer cell
 * 0x1F800044 and calls func_8014D978. That state byte is the value the overlay
 * dispatch at 0x800C18F0 scales by four onto its 0x800C1B18 run and the byte
 * dispatcher at 0x800C18A0 scales onto its 0x800C1B0C run, and this body's own
 * address word 0x800C1934 is entry 3 of that 0x800C1B0C run and entry 0 of the
 * 0x800C1B18 run; those two stored table words are its only in-image
 * references, so no jal names it and the overlay's two dispatchers reach it
 * through them. The body is instruction-for-instruction identical to the
 * already-exact sibling publisher at 0x800C1848 of this payload, differing only
 * in the three addresses it installs. Both prologue and return path place the
 * boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetStateTo2BossBoss03016_800C1934(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1AC0;
  FIELD_REF(Boss03016StepHandler, work, 0xE4u) =
      dispatchIndexedHandlerBossBoss03016_800C198C;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1AD4;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
