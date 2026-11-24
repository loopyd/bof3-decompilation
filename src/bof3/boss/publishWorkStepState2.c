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

/* This payload's byte-indexed handler dispatcher at 0x800C19C8: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C1FF4
 * run that this publisher writes into the work record's handler slot. */
void callIndexedHandler1(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step-0 setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1F9C into +0xEC, the byte dispatcher
 * callIndexedHandler1 into +0xE4 and the in-row data
 * pointer 0x800C1FAC (0x800C1F9C + 0x10) into +0xE8 -- then stores 2 into the
 * state byte at +1 of the work record the main program publishes at the
 * scratchpad pointer cell 0x1F800044 and calls func_8014D978. That state byte is
 * the value the overlay's entry dispatcher at 0x800C1804 scales by four against
 * its handler run at 0x800C1FBC, and this function is that run's entry 0 (the
 * word at 0x800C1FBC), so writing 2 makes the next dispatch call the
 * main-program handler 0x801E35D8 instead of re-entering this installer. No jal
 * names 0x800C1848 and the run word is its only in-image reference, so the
 * overlay entry dispatcher reaches it. Compared with the payload's other
 * publisher at 0x800C1AEC, whose source gated exact, this 0x58-byte span differs
 * in one word only, the low immediate of the dispatcher materialization (0x1C08
 * there, 0x19C8 here); this seed copied that body's statement order, operand
 * forms and declaration style and writes this payload's own literals. Both
 * prologue and return path place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepState2(
    void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1F9C;
  FIELD_REF(Boss00216StepHandler, work, 0xE4u) =
      callIndexedHandler1;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1FAC;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
