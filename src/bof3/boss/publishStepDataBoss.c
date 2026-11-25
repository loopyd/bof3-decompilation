#include "bof3/bof3.h"

typedef void (*Boss03316StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C2744 `textbin` row, the 0x94 bytes the reviewed Splat layout places at
 * 0x800C2744 through 0x800C27D7. 0x800C2744 is that row's own label;
 * 0x800C2750 is its +0xC word, inside the row and not a row boundary, so it is
 * referenced as its own data symbol rather than as a row start. That word is the
 * halfword pair effect table the byte-1 handler at 0x800C1A58 reads back from
 * work record offset 0xE8. */
extern u8 D_800C2744; /* @source 0x800C2744 @kind data */
extern u8 D_800C2750; /* @source 0x800C2750 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and the battle overlays read back; this publisher writes its three handler
 * words. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Scratchpad work-object pointer cell: the battle code publishes the scratch
 * work record here, and its byte at +1 is the dispatch state the overlay entry
 * dispatcher callStateTable275Boss reads. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* This payload's byte-indexed handler dispatcher at 0x800C1CA4: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C27A0
 * handler run. */
void dispatchIndexRun27a0(u8 index);

/* Shared main-program helper the boss archive family calls at the end of its
 * step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step-0 setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C2744 into +0xEC, the handler dispatcher
 * dispatchIndexRun27a0 into +0xE4 and the data pointer
 * 0x800C2750 (0x800C2744 + 0xC, inside that same row) into +0xE8 -- then stores
 * 2 into the state byte at +1 of the work record the main program publishes at
 * the scratchpad pointer cell 0x1F800044 and calls func_8014D978. That state
 * byte is the value callStateTable275Boss indexes
 * its code-pointer run by, and this function is that run's entry 0 (the word at
 * 0x800C275C), so writing 2 selects its third entry, 0x801E35D8, instead of
 * re-entering this installer; no jal reaches it and that table word is its only
 * in-image reference, so the overlay entry calls it. The state byte store fills
 * the call's delay slot and the 0x18-byte frame only keeps $ra at 0x10.
 * The three stored addresses are the only difference from the publisher at
 * 0x800C1848 of BIN/BOSS/BOSS028.EMI#16, BIN/BOSS/BOSS015.EMI#16,
 * BIN/BOSS/BOSS002.EMI#16 and BIN/BOSS/BOSS030.EMI#16, exact siblings whose
 * 0x58-byte spans compare equal word for word except the three low immediates of
 * these pointer materializations, 0x2140/0x1AF0/0x214C, 0x199C/0x18A0/0x19A8,
 * 0x1F9C/0x19C8/0x1FAC and 0x1AB4/0x18A0/0x1ACC there; this seed copies their
 * statement order, operand forms and declaration style and writes this payload's
 * own literals. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishStepDataBoss(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C2744;
  FIELD_REF(Boss03316StepHandler, work, 0xE4u) =
      dispatchIndexRun27a0;
  FIELD_REF(void *, work, 0xE8u) = &D_800C2750;
  D_1F800044[1] = 2;
  func_8014D978();
}
