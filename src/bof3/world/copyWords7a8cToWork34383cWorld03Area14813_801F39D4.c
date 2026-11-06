#include "bof3/bof3.h"

extern u32 D_80147A8C; /* @source 0x80147A8C @kind data */
extern u32 D_80147A90; /* @source 0x80147A90 @kind data */
extern u32 D_80147A94; /* @source 0x80147A94 @kind data */

/* @source 0x801F39D4
 * @behavior Copies the words at 0x80147A8C, 0x80147A90 and 0x80147A94 into
 * offsets 0x34, 0x38 and 0x3C of the work record published in scratchpad
 * pointer slot 0x44, in that order. The single cell load serves all three
 * stores, each global is read through its own lui/lw pair whose load delay the
 * original leaves as a nop, and the last store sits in the return delay slot.
 * Takes no arguments and returns nothing.
 *
 * Name evidence: the role comes from this body's own three stores and the
 * resident words they read, and the name follows the already-exact
 * cross-target sibling that carries the same 0x38 bytes in world04/area167/13
 * at 0x801F4434, whose metadata records the same three-way copy. No in-image
 * caller exists; the overlay's own dispatch tables reach their entries, so an
 * unindexed setter of this shape is ordinary for this payload.
 *
 * Shape evidence: this range is byte-identical (56 bytes, verified against
 * both payloads at their own load offsets) to that exact sibling
 * copyWords7a8cToWork34383cWorld04Area16713_801F4434, so its statement order,
 * scratchpad accessor spelling, source-local extern style and offset
 * arithmetic are copied unchanged; the literals are the same shared resident
 * addresses, and the symbol spelling is this target's own declaration.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWords7a8cToWork34383cWorld03Area14813_801F39D4(void) {
  u8* work;

  work = SPAD_PTR_SLOT(u8, 0x44);
  *(u32*)(work + 0x34) = D_80147A8C;
  *(u32*)(work + 0x38) = D_80147A90;
  *(u32*)(work + 0x3C) = D_80147A94;
}
