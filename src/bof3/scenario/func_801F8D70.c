#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F8D70
 * @behavior Runs the overlay's following beam entry drawLinkedRecordCursorBeam
 * with the same no-argument image and returns: this 8-instruction wrapper
 * passes nothing to the callee, neither reads nor writes overlay state of its
 * own, and keeps a real jal (a frame saving the return address) instead of a
 * sibling tail call, exactly as the original does. The address is also carried
 * as a code pointer by the data of the concurrently loaded game/00 overlay, so
 * it is an externally referenced entry rather than an internal label. Takes no
 * arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F8D70(void) {
  drawLinkedRecordCursorBeam();
}
