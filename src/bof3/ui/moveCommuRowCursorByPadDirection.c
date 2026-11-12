#include "bof3/bof3.h"

/* Shared pad-state word: the pressed buttons of the current frame. */
extern u16 D_80145AA8; /* @source 0x80145AA8 @kind data */

/* Commu request bytes of this overlay's handlers, which the row-cursor stepper
 * also carries its press state in: 0x801D4DF4 is the base row a press started
 * from, 0x801D4DFC the press state (0 idle, 1 pressed, 2 repeating) and
 * 0x801D4DF8 the hold flag, which also holds the applied offset of the two
 * wide moves. The confirm path snapshots all three. */
extern u8 D_801D4DF4; /* @source 0x801D4DF4 @kind data */
extern u8 D_801D4DF8; /* @source 0x801D4DF8 @kind data */
extern u8 D_801D4DFC; /* @source 0x801D4DFC @kind data */

/* @source 0x8014E640 @kind unknown
 * Shared pad-mask decoder: returns the direction bits of the button mask it is
 * given.
 */
extern s32 func_8014E640(u32 mask);

/* @source 0x8015DF18 @kind unknown
 * Resident executable sound-cue dispatcher; consumes one u32 cue ID.
 */
extern void dispatchSoundCue(u32 cue_id);

/* @source 0x801D3B0C
 * @behavior Steps the 8-bit row cursor the argument points at by the pad
 * direction the shared decoder at 0x8014E640 reports for the 0xF000 bits of the
 * shared pad-state word 0x80145AA8, keeping the press state in the commu
 * request bytes 0x801D4DFC, 0x801D4DF8 and 0x801D4DF4. Bit 0x1000 of the
 * direction: state 0 sets hold flag 1, state 1 and stores the base row
 * 0x801D4DF4 plus 4 in the cursor; state 1 clears the cursor and the state when
 * the hold flag is clear and otherwise clears the hold flag and steps the cursor
 * back by 4; state 2 steps the cursor down by one and pins it to 0xB when the
 * result is below 9. Bit 0x4000: state 0 clears the hold flag, sets state 1
 * and stores the base row in the cursor; state 1 clears the cursor and the state
 * when the hold flag is set and otherwise sets the hold flag and steps the
 * cursor forward by 4; state 2 steps the cursor up by one and pins it to 9 when
 * the result is not below 0xC. Bit 0x8000: state 0 sets state 2 and stores 9 in
 * the cursor; state 1 steps the base row and the cursor down by one and, once
 * the base row reaches 0, pins it to 1, sets state 2 and stores the hold flag
 * plus 0xA in the cursor; state 2 clears the state and the cursor when the
 * cursor is 9 and otherwise sets state 1, stores the cursor minus 0xA in the
 * hold flag, stores four times the cursor minus 0x24 in the cursor and pins the
 * base row to 4. Bit 0x2000: state 0 sets state 2 and stores 9 in the cursor;
 * state 1 steps the base row and the cursor up by one and, once the base row
 * reaches 5, pins it to 4, sets state 2 and stores the hold flag plus 0xA in the
 * cursor; state 2 clears the state and the cursor when the cursor is 9 and
 * otherwise sets state 1, pins the base row to 1, stores the cursor minus 0xA in
 * the hold flag and stores four times that same difference plus one in the
 * cursor. Every state value other than 0, 1 and 2 moves nothing, and any
 * nonzero direction mask queues resident cue 0x100 through the shared cue
 * dispatcher at 0x8015DF18. Its 0x18-byte frame only holds ra and s0.
 * @status exact
 * @match 100.00
 * @residual none
 */
void moveCommuRowCursorByPadDirection(u8 *rowPtr) {
  s32 direction;

  direction = func_8014E640(D_80145AA8 & 0xF000);
  if (direction & 0x1000) {
    switch (D_801D4DFC) {
    case 0:
      D_801D4DF8 = 1;
      D_801D4DFC = 1;
      *rowPtr = D_801D4DF4 + 4;
      break;
    case 1:
      if (D_801D4DF8 == 0) {
        D_801D4DFC = 0;
        *rowPtr = 0;
      } else {
        D_801D4DF8 = 0;
        *rowPtr = *rowPtr - 4;
      }
      break;
    case 2:
      *rowPtr = *rowPtr - 1;
      if (*rowPtr < 9) {
        *rowPtr = 0xB;
      }
      break;
    }
  } else if (direction & 0x4000) {
    switch (D_801D4DFC) {
    case 0:
      D_801D4DF8 = 0;
      D_801D4DFC = 1;
      *rowPtr = D_801D4DF4;
      break;
    case 1:
      if (D_801D4DF8 != 0) {
        D_801D4DFC = 0;
        *rowPtr = 0;
      } else {
        D_801D4DF8 = 1;
        *rowPtr = *rowPtr + 4;
      }
      break;
    case 2:
      *rowPtr = *rowPtr + 1;
      if (*rowPtr >= 0xC) {
        *rowPtr = 9;
      }
      break;
    }
  } else if (direction & 0x8000) {
    switch (D_801D4DFC) {
    case 0:
      D_801D4DFC = 2;
      *rowPtr = 9;
      break;
    case 1:
      D_801D4DF4 = D_801D4DF4 - 1;
      *rowPtr = *rowPtr - 1;
      if (D_801D4DF4 == 0) {
        D_801D4DF4 = 1;
        D_801D4DFC = 2;
        *rowPtr = D_801D4DF8 + 0xA;
      }
      break;
    case 2:
      if (*rowPtr == 9) {
        D_801D4DFC = 0;
        *rowPtr = 0;
      } else {
        D_801D4DFC = 1;
        D_801D4DF8 = *rowPtr - 0xA;
        *rowPtr = (*rowPtr << 2) - 0x24;
        D_801D4DF4 = 4;
      }
      break;
    }
  } else if (direction & 0x2000) {
    switch (D_801D4DFC) {
    case 0:
      D_801D4DFC = 2;
      *rowPtr = 9;
      break;
    case 1:
      D_801D4DF4 = D_801D4DF4 + 1;
      *rowPtr = *rowPtr + 1;
      if (D_801D4DF4 >= 5) {
        D_801D4DF4 = 4;
        D_801D4DFC = 2;
        *rowPtr = D_801D4DF8 + 0xA;
      }
      break;
    case 2:
      if (*rowPtr == 9) {
        D_801D4DFC = 0;
        *rowPtr = 0;
      } else {
        D_801D4DFC = 1;
        D_801D4DF4 = 1;
        D_801D4DF8 = *rowPtr - 0xA;
        *rowPtr = (*rowPtr - 0xA) * 4 + 1;
      }
      break;
    }
  }
  if (direction & 0xF000) {
    dispatchSoundCue(0x100);
  }
}
