#include "bof3/bof3.h"

typedef void (*Boss00816StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C2640 data row, the 320 bytes the Splat `textbin` row places at the end
 * of the segment. 0x800C264C is that row's +0xC word, the eight-byte 0x00
 * through 0x07 index run, and 0x800C2658 is its +0x18 word, the two 0x0600
 * halfwords; both sit inside the row and not on a row boundary, so each is
 * referenced as its own data symbol rather than as a row start. */
extern u8 D_800C264C; /* @source 0x800C264C @kind data */
extern u8 D_800C2658; /* @source 0x800C2658 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed step dispatcher at 0x800C1C88: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C26C0
 * run, whose entries 0 through 2 are the noop handlers at 0x800C1CC0,
 * 0x800C1CC8 and 0x800C1CD0 and whose entry 3 is the publisher at 0x800C1D1C. */
void dispatchIndexedHandlerBossBoss00816_800C1C88(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step-0 setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * block pointer 0x800C264C into +0xEC, the byte dispatcher
 * dispatchIndexedHandlerBossBoss00816_800C1C88 into +0xE4 and the in-row data
 * pointer 0x800C2658 into +0xE8 -- then stores 2 into the state byte at +1 of
 * the work record the main program publishes at the scratchpad pointer cell
 * 0x1F800044 and calls func_8014D978. That state byte is the value the overlay
 * dispatcher callStateTable2660Boss scales by four
 * against the handler run based at 0x800C2660, and this function is that run's
 * entry 0 (the word at 0x800C2660), so writing 2 makes the next dispatch call
 * the main-program handler 0x801E35D8 instead of re-entering this installer. No
 * jal names 0x800C1848 and the run word is its only in-image reference, so the
 * overlay entry dispatcher reaches it. The body re-reads the pointer cell for
 * the state byte after the three stores. Compared with the payload's other
 * publisher at 0x800C1D1C, whose source gated exact, this 0x58-byte span
 * differs in one word only, the low immediate of the dispatcher
 * materialization (0x1C88 here, 0x1D74 there); this seed copied that body's
 * statement order, operand forms and declaration style and writes this
 * payload's own literals. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishStepDataSet(
    void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C264C;
  FIELD_REF(Boss00816StepHandler, work, 0xE4u) =
      dispatchIndexedHandlerBossBoss00816_800C1C88;
  FIELD_REF(void *, work, 0xE8u) = &D_800C2658;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
