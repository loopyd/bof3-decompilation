#include "bof3/bof3.h"

typedef void (*Boss00816StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C2640 data row, the 320 bytes the Splat `textbin` row places at the end
 * of the segment. 0x800C264C is that row's +0xC word and 0x800C2658 is its
 * +0x18 word, so each is referenced as its own data symbol rather than as a row
 * start. */
extern u8 D_800C264C; /* @source 0x800C264C @kind data */
extern u8 D_800C2658; /* @source 0x800C2658 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed no-op run dispatcher at 0x800C1E60: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C2738
 * run, whose three words are the inert handlers 0x800C1E98, 0x800C1EA0 and
 * 0x800C1EA8. */
void dispatchIndexedHandlerBossBoss00816_800C1E60(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D1C of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1E08
 * @behavior Step-3 handler of this overlay's work state machine and entry 0 of
 * the 0x800C2708 run: index 3 of the 0x800C26FC run that the step dispatcher
 * dispatchIndexedHandlerBossBoss00816_800C1D74 selects from. It publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * block pointer 0x800C264C into +0xEC, the no-op-run step handler
 * dispatchIndexedHandlerBossBoss00816_800C1E60 into +0xE4 and the in-row data
 * pointer 0x800C2658 into +0xE8 -- then stores 2 into the state byte at +1 of
 * the work record the main program publishes at the scratchpad pointer cell
 * 0x1F800044 and calls func_8014D978. Instruction for instruction identical to
 * the exact sibling publisher at 0x800C1D1C of this same payload apart from the
 * handler address stored into +0xE4, which is the only differing immediate the
 * two bodies carry; both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishNoopStepHandlerAndDataThenSetStateTo2BossBoss00816_800C1E08(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C264C;
  FIELD_REF(Boss00816StepHandler, work, 0xE4u) =
      dispatchIndexedHandlerBossBoss00816_800C1E60;
  FIELD_REF(void *, work, 0xE8u) = &D_800C2658;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
