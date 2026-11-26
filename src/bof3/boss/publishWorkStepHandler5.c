#include "bof3/bof3.h"

typedef void (*Boss02416StepHandler)(u8 index);

/* The two data pointers this publisher installs come from this payload's
 * 0x800C1B44 `textbin` row, the 140 bytes the reviewed Splat layout places at
 * 0x800C1B44 through the 0x800C1BD0 that ends the segment. That row holds this
 * overlay's 0x800C1B58 state run, the 0x800C1B88 dispatcher run and the record
 * view the work record points at. 0x800C1B44 is the row's own label; 0x800C1B50
 * is its +0xC word, the 0x0600 followed by 0xFFFF halfword pair inside the row
 * and not a row boundary, so it is referenced as its own data symbol rather than
 * as a row start. */
extern u8 D_800C1B44; /* @source 0x800C1B44 @kind data */
extern u8 D_800C1B50; /* @source 0x800C1B50 @kind data */

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back, the same cell the sibling publishers of
 * BIN/BOSS/BOSS028.EMI#16 and BIN/BOSS/BOSS030.EMI#16 write. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* This payload's byte-indexed step dispatcher at 0x800C18A0: it masks its
 * argument with 0xFF, scales it by four and calls the entry of the 0x800C1B88
 * run, whose entries 0 through 2 are the noop handlers at 0x800C18D8,
 * 0x800C18E0 and 0x800C18E8 and whose entry 3 is the span at 0x800C1934. */
void dispatchIndexedHandler6(u8 index);

/* Shared main-program helper the whole boss archive family calls at the end of
 * its step setup; it takes no arguments and returns none here. */
void func_8014D978(void);

/* @source 0x800C1848
 * @behavior Step-0 setup of this overlay's work state machine: publishes three
 * words into the current enemy work record loaded from 0x801EB4E8 -- the data
 * row pointer 0x800C1B44 into +0xEC, the step dispatcher
 * dispatchIndexedHandler6 into +0xE4 and the data pointer
 * 0x800C1B50 (0x800C1B44 + 0xC, inside that same row) into +0xE8 -- then stores
 * 2 into the state byte at +1 of the work record the main program publishes at
 * the scratchpad pointer cell 0x1F800044 and calls func_8014D978. That state
 * byte is the value callStateByte1Table523 scales by
 * four against its 0x800C1B58 run, whose entry 0 is the word 0x800C1848, so
 * writing 2 selects that run's entry 2, 0x801E35D8, instead of re-entering this
 * installer; a scan of the payload finds the word 0x800C1848 at 0x800C1B58 only
 * and no jal names it, so the overlay entry does. The state byte store fills the
 * call's delay slot and the 0x18-byte frame only keeps $ra at 0x10. Compared
 * with the publisher at 0x800C1848 of BIN/BOSS/BOSS030.EMI#16, whose 0x58-byte
 * span differs from this one in two words only, the low immediates 0x1B44 for
 * 0x1AB4 and 0x1B50 for 0x1ACC of those pointer materializations, this seed
 * copies that body's statement order, operand forms and declaration style and
 * writes this payload's own literals. Both prologue and return path place the
 * boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishWorkStepHandler5(void) {
  u8 *work = D_801EB4E8;

  FIELD_REF(void *, work, 0xECu) = &D_800C1B44;
  FIELD_REF(Boss02416StepHandler, work, 0xE4u) =
      dispatchIndexedHandler6;
  FIELD_REF(void *, work, 0xE8u) = &D_800C1B50;
  SPAD_PTR_SLOT(u8, 0x44u)[1] = 2;
  func_8014D978();
}
