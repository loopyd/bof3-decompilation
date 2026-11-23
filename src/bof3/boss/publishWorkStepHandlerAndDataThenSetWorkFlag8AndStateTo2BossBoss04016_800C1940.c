#include "bof3/bof3.h"

typedef void (*Boss04016StepHandler)(u8 index);

typedef struct Boss04016PublishWork {
  u8 unk_00[0xE4];
  Boss04016StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss04016PublishWork;

/* This payload's data row at 0x800C1B1C, the 0x8C-byte run the Splat `textbin`
 * row places after this payload's code. Its first three words are the small
 * record 0x02020101, 0x03030404 and 0x00000101, its +0xC word is 0xFFFF0600
 * followed by 0xFFFFFFFF, and the eleven words from 0x800C1B30 are the code
 * pointers the 0x800C1804 state dispatcher indexes with the work state byte at
 * +1. 0x800C1B28 is that record's +0xC word, inside the row and not a row
 * boundary, so it is referenced as its own data symbol rather than as a row
 * start. */
extern u8 D_800C1B1C; /* @source 0x800C1B1C @kind data */
extern u8 D_800C1B28; /* @source 0x800C1B28 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; 0x801E24F8 of BIN/BATTLE/BATTLE.EMI#03
 * reads it as one of the two cells that name the current work record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed handler dispatcher at 0x800C19A4, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C1B9C table, whose three slots are this
 * overlay's closing inert handlers 0x800C19DC, 0x800C19E4 and 0x800C19EC. It
 * takes the slot the earlier installer at 0x800C1848 filled with the 0x800C18AC
 * dispatcher. */
void dispatchIndexedHandlerBossBoss04016_800C19A4(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1C68 and 0x800C1D40 of BIN/BOSS/BOSS008.EMI
 * and BIN/BOSS/BOSS034.EMI; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1940
 * @behavior Step setup of this overlay's work state machine, the second of its
 * two publishers: writes three words into the current enemy work record loaded
 * from 0x801EB4E8 -- the data row pointer 0x800C1B1C into +0xEC, the
 * byte-indexed dispatcher dispatchIndexedHandlerBossBoss04016_800C19A4 into
 * +0xE4 and the in-row pointer 0x800C1B28 (0x800C1B1C + 0xC) into +0xE8 -- then
 * raises bit 0x8 of the record +0x104 flag word, stores 2 into the state byte at
 * +1 of the work record the main program publishes at the scratchpad pointer
 * cell 0x1F800044 and calls func_8014D978. No jal names 0x800C1940: the only
 * in-image word that does is 0x800C1B6C, entry 0 of the twelve-pointer run based
 * there that this payload's 0x800C18FC dispatcher indexes with that same state
 * byte, so writing 2 moves the next such dispatch to entry 2, the shared
 * main-program handler 0x801E35D8, instead of re-entering this installer. No
 * word of the payload holds this address in any other form, and nothing here
 * reads the +0x104 word back.
 *
 * Shape: the 0x64-byte span equals this payload's exact sibling at 0x800C1848
 * in 98 of 100 bytes and differs from it only in the two low bytes of the
 * dispatcher materialization at +0x20 (0x19A4 here, 0x18AC there, one 0x8000
 * page), so that sibling supplied the statement order, the operand forms and the
 * declaration style while this span's own bytes supplied the handler and every
 * other literal. The same 100 bytes sit byte-identical at 0x800C1940 of
 * BIN/BOSS/BOSS049.EMI#16, a payload whose row for that span was still an asm
 * row there and so offered no source; that equality confirmed the span and the
 * name, and the clone adapter stays declined because no lifted body replaces
 * these bytes. Prologue, return path and the trailing nop place the boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss04016_800C1940(
    void) {
  Boss04016PublishWork *work = (Boss04016PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C1B1C;
  work->step_handler = dispatchIndexedHandlerBossBoss04016_800C19A4;
  work->step_data = &D_800C1B28;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
