#include "bof3/bof3.h"

typedef void (*Boss03416StepHandler)(u8 index);

/* This overlay's reviewed data row at 0x800C26B8, the `textbin` row that ends
 * the segment and spans 0x800C26B8 through 0x800C277B. This function publishes
 * the row base into the work record. */
extern u8 D_800C26B8; /* @source 0x800C26B8 @kind data */

/* Word +0x0C of that same row, the byte run 0x00 0x06 0x00 0x06 followed by
 * 0xFFFFFFFF, inside the five-word record that opens the row: not a row
 * boundary, so it is declared at its own address rather than as a second row.
 * The row's own dispatchers name the interior words 0x800C26CC, 0x800C26FC,
 * 0x800C2714 and 0x800C2730 the same way. */
extern u8 D_800C26C4; /* @source 0x800C26C4 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its overlays read back to install their step handler and data. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed handler dispatcher at 0x800C1D60: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C2730
 * handler run. */
void callIndexedHandler37(u8 index);

/* Shared main-program helper the boss archive family calls at the end of its
 * step setup, reached here through one jal at 0x800C18A0. */
void func_8014D978(void);

/* @source 0x800C1860
 * @behavior Step-0 setup of this overlay's work state machine. Loads the current
 * enemy work record from 0x801EB4E8 and publishes three words into it: the data
 * row pointer 0x800C26B8 into +0xEC, the byte-indexed handler dispatcher
 * 0x800C1D60 into +0xE4 and the in-row data word 0x800C26C4 into +0xE8. It then
 * loads the work record the main program publishes at the scratchpad pointer
 * cell 0x1F800044, stores 2 into that record's state byte at offset 1 and calls
 * func_8014D978. That state byte is the value this overlay's byte-1 dispatcher
 * dispatchByte1Table26cc scales by four against the
 * handler run at the 0x800C26CC interior word of the same row, and that run's
 * entry 0 is this function's own address, so writing 2 moves the next dispatch
 * off this installer to 0x801E35D8, a main-program handler. No jal names
 * 0x800C1860 and the run word at 0x800C26CC is its only in-image reference, so
 * the overlay entry reaches it and the entry takes no arguments.
 *
 * Byte evidence: the 0x58-byte span compares word for word with the body at the
 * same address of BIN/BOSS/BOSS021.EMI#16 apart from three `addiu` %lo
 * immediates - the two published data pointers, 0x1CB0 there against 0x26B8
 * here and 0x1CC8 there against 0x26C4 here, and the dispatcher, 0x198C there
 * against 0x1D60 here - so the statement order, operand forms and declaration
 * style follow that seed while every literal is written from these bytes. Both
 * prologue and return path place the boundary, and a search of every payload
 * under out/binaries for this 22-word body finds no byte-identical copy, so the
 * seat is sole evidence and no clone applies.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepPair(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C26B8;
  FIELD_REF(Boss03416StepHandler, work, 0xE4u) =
      callIndexedHandler37;
  FIELD_REF(void *, work, 0xE8u) = &D_800C26C4;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
