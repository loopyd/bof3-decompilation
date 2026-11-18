#include "bof3/bof3.h"

typedef void (*Boss02816StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C2140 textbin row, the 64 bytes the reviewed Splat layout places at
 * 0x800C2140 through the 0x800C217F that ends where the 0x800C2180 row begins.
 * 0x800C2140 is that row's own label; 0x800C214C is its +0xC word, inside the
 * row and not a row boundary, so it is referenced as its own data symbol rather
 * than as a row start. */
extern u8 D_800C2140; /* @source 0x800C2140 @kind data */
extern u8 D_800C214C; /* @source 0x800C214C @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back, the same cell the exact sibling publisher
 * of BIN/BOSS/BOSS030.EMI#16 writes. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed step dispatcher at 0x800C1AF0: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C2198
 * handler run. */
void dispatchIndexedHandlerBossBoss02816_800C1AF0(u8 index);

/* Shared main-program helper the boss archive family calls at the end of its
 * step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step-0 setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C2140 into +0xEC, the step dispatcher
 * dispatchIndexedHandlerBossBoss02816_800C1AF0 into +0xE4 and the data pointer
 * 0x800C214C (0x800C2140 + 0xC, inside that same row) into +0xE8 -- then stores
 * 2 into the state byte at +1 of the work record the main program publishes at
 * the scratchpad pointer cell 0x1F800044 and calls func_8014D978. That state
 * byte is the value dispatchStateByte1Table2154BossBoss02816_800C1804 indexes
 * its eleven-word 0x800C2154 run by, and this function is that run's entry 0
 * (the word at 0x800C2154), so writing 2 selects its third entry, 0x801E35D8,
 * instead of re-entering this installer; no jal reaches it and the table word is
 * its only in-image reference, so the overlay entry does. The three stored
 * addresses are the only difference from the publisher at 0x800C1848 of
 * BIN/BOSS/BOSS030.EMI#16, an exact sibling whose 0x58-byte span compares equal
 * word for word except the three low immediates of these pointer
 * materializations, 0x1AB4, 0x18A0 and 0x1ACC there; this seed copies its
 * statement order, operand forms and declaration style and writes this payload's
 * own literals. Both prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetStateTo2BossBoss02816_800C1848(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C2140;
  FIELD_REF(Boss02816StepHandler, work, 0xE4u) = dispatchIndexedHandlerBossBoss02816_800C1AF0;
  FIELD_REF(void *, work, 0xE8u) = &D_800C214C;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
