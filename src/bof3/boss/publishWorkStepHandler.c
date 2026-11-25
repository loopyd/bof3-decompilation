#include "bof3/bof3.h"

typedef void (*Boss03113StepHandler)(u8 index);

/* This payload's last data row: the 0x68-byte `textbin` run the Splat layout
 * keeps from 0x800C1AB8 to the end of the segment at 0x800C1B20. This function
 * publishes the row base into the work record. */
extern u8 D_800C1AB8; /* @source 0x800C1AB8 @kind data */

/* Word +0x0C of that same row, the halfword run the work record keeps beside it
 * and not a row boundary, so it is declared at its own address rather than as a
 * second row. The dispatcher source names the row interior at 0x800C1ACC and
 * 0x800C1B14 the same way. */
extern u8 D_800C1AC4; /* @source 0x800C1AC4 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its overlays read back to install their step handler and data. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed handler dispatcher: the C source that defines it
 * carries 0x800C1978 as its origin, and the field this function writes holds
 * the address it publishes. */
void dispatchIndexTable1b14(u8 index);

/* Shared main-program helper the boss archive family calls at the end of its
 * step setup, reached here through one jal at 0x800C1888. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step-0 setup of this overlay's work state machine. Loads the current
 * enemy work record from 0x801EB4E8 and publishes three words into it: the data
 * row pointer 0x800C1AB8 into +0xEC, the byte-indexed handler dispatcher
 * 0x800C1978 into +0xE4 and the in-row data word 0x800C1AC4 into +0xE8. It then
 * loads the work record the main program publishes at the scratchpad pointer
 * cell 0x1F800044, stores 2 into that record's state byte at offset 1 and calls
 * func_8014D978. That state byte is the value this overlay's entry dispatcher at
 * 0x800C1804 scales by four against its eighteen-word handler run at 0x800C1ACC,
 * whose entry 0 is this function, so writing 2 moves the next dispatch to the
 * main-program handler 0x801E35D8 instead of re-entering this installer. No
 * jal names 0x800C1848 and the run word at 0x800C1ACC is its only in-image
 * reference, so the overlay entry dispatcher reaches it and the entry takes no
 * arguments.
 *
 * Byte evidence: the 0x58-byte span compares equal word for word with the
 * already-exact installer publishWorkStepHandlerAndDataThenSetStateTo2BossBoss00216_800C1848
 * at the same address of BIN/BOSS/BOSS002.EMI#16 except three `addiu` %lo
 * immediates - the two published data pointers, 0x1F9C there against 0x1AB8 here
 * and 0x1FAC there against 0x1AC4 here, and the dispatcher, 0x19C8 there against
 * 0x1978 here - so the statement order, operand forms and declaration style
 * follow that seed while every literal is written from these bytes. Both
 * prologue and return path place the boundary, and a search of every payload
 * under out/binaries for this 22-word body finds no byte-identical copy, so the
 * seat is sole evidence and no clone applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandler(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1AB8;
  FIELD_REF(Boss03113StepHandler, work, 0xE4u) =
      dispatchIndexTable1b14;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1AC4;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
