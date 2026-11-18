#include "bof3/bof3.h"

/* @source 0x801F2C90
 * @behavior Advances the scratchpad work-object byte at offset 0x09 by two at
 * byte width. The work pointer is taken from the pointer cell at 0x1F800044
 * once and the same register serves both the byte load and the byte store, so
 * this body keeps a single cell load where its neighbour at 0x801F2C60
 * re-materializes the cell for each of its two accesses. The span is entry 1
 * of the two-word handler run at the table base 0x801F3554, whose entry 0 is
 * 0x801F2C60; the overlay entry at 0x801F2C04 indexes that run with the work
 * object's byte 2 scaled by four before dispatching, so byte 0x09 is the step
 * byte this handler advances before 0x801F2CB0 consumes it. Takes no arguments
 * and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkByte9By2World03Area14213_801F2C90(void) {
  u8 **slots;
  u8 *work;

  slots = SPAD_PTR_TABLE(u8);
  work = slots[0x11];
  work[9] += 2;
}
