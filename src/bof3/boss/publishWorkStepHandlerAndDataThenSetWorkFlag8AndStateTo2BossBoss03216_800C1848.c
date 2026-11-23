#include "bof3/bof3.h"

typedef void (*Boss03216StepHandler)(u8 index);

typedef struct Boss03216PublishWork {
  u8 unk_00[0xE4];
  Boss03216StepHandler step_handler;
  void *step_data;
  void *data_row;
  u8 unk_F0[0x14];
  u32 flags;
} Boss03216PublishWork;

/* This payload's data row at 0x800C1C2C, the 0x68-byte run the Splat `textbin`
 * row places after this payload's code: its opening words are 0x00800282,
 * 0x04840181, 0x00800000 and 0x0006FFFF, then the word 0xFFFFFFFF, and the
 * sixteen code pointers from 0x800C1C40 that this payload's state dispatcher at
 * 0x800C1804 indexes are 0x800C1848, 0x801E52E8, 0x801E35D8, 0x801E3638,
 * 0x801E3980, 0x801E3A00, 0x800C18AC, 0x801E4AE8, 0x801E4D8C, 0x801E4F64,
 * 0x801E52E8, 0x801E5288, 0x801E3BAC, 0x801E3BD0, 0x801E3BAC and 0x801E4490.
 * 0x800C1C38 is that row's +0xC word, inside the row and not a row boundary, so
 * it is referenced as its own data symbol rather than as a row start. The
 * generated undefined_syms_auto.txt binds both words, so these declarations
 * need no map row.
 */
extern u8 D_800C1C2C; /* @source 0x800C1C2C @kind data */
extern u8 D_800C1C38; /* @source 0x800C1C38 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back; this target's generated
 * undefined_syms_auto.txt already binds the address, and every access here is
 * offset-anchored on that pointer rather than on a modelled record. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed handler dispatcher at 0x800C1978, the handler this
 * publisher installs at +0xE4: it masks its argument with 0xFF, scales it by
 * four and calls the entry of the 0x800C1C88 run. */
void dispatchIndexedHandlerBossBoss03216_800C1978(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup, for example 0x800C1848 of BIN/BOSS/BOSS040.EMI#16 and
 * BIN/BOSS/BOSS037.EMI#16; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step setup of this overlay's work state machine, the 0x64-byte span
 * at payload offset 0x48 that opens with the 0x18-byte frame reservation and
 * closes with the return at 0x800C18A4: publishes three words into the current
 * enemy work record loaded from the main-RAM pointer cell 0x801EB4E8 -- the data
 * row pointer 0x800C1C2C into +0xEC, the indexed dispatcher
 * dispatchIndexedHandlerBossBoss03216_800C1978 into +0xE4 and the in-row pointer
 * 0x800C1C38 (0x800C1C2C + 0xC) into +0xE8 -- then raises bit 0x8 of the record
 * +0x104 flag word, stores 2 into the state byte at +1 of the work record the
 * main program publishes at the scratchpad pointer cell 0x1F800044 and calls
 * func_8014D978. No jal names 0x800C1848: its only in-image reference is the
 * word 0x800C1848 stored at 0x800C1C40, entry 0 of the run that this payload's
 * state dispatcher at 0x800C1804 indexes with that same state byte, so writing 2
 * moves the next dispatch to that run's entry 2, the shared handler 0x801E35D8,
 * instead of re-entering this installer. Nothing in this payload reads the
 * +0x104 word back. The record pointer load precedes frame creation and the state
 * byte store fills the call's delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: no payload under out/binaries holds a byte-identical copy of
 * this span -- a scan of all 398 extracted payloads for these 0x64 bytes hit
 * none, and a twin would have to carry this payload's own dispatcher word
 * 0x800C1978 and row base 0x800C1C2C, so the clone lever is declined. The exact
 * sibling at 0x800C1848 of BIN/BOSS/BOSS040.EMI#16 (also 0x800C1848 of
 * BIN/BOSS/BOSS037.EMI#16) compares equal word for word except the three addiu
 * low immediates that materialize the data row (0x1C2C here, 0x1B1C there), the
 * dispatcher (0x1978 here, 0x18AC there) and the in-row pointer (0x1C38 here,
 * 0x1B28 there); every other word, including the shared func_8014D978 jal, is
 * equal. That sibling supplies the statement order, the record struct shape and
 * the operand forms; this span's own bytes supply every literal.
 */
void publishWorkStepHandlerAndDataThenSetWorkFlag8AndStateTo2BossBoss03216_800C1848(
    void) {
  Boss03216PublishWork *work = (Boss03216PublishWork *)D_801EB4E8;
  u8 *state;

  work->data_row = &D_800C1C2C;
  work->step_handler = dispatchIndexedHandlerBossBoss03216_800C1978;
  work->step_data = &D_800C1C38;
  state = SPAD_PTR_SLOT(u8, 0x44u);
  work->flags |= 8u;
  state[1] = 2;
  func_8014D978();
}
