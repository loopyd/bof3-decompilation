#include "bof3/scenario/scena00_internal.h"

/* @source 0x801F878C
 * @behavior Seeds the scratchpad work object published at 0x1F800044 for the
 * handler-table entry that owns this function: stores the work object's two
 * 16-bit cursor coordinates at offsets 0x2E and 0x30, each being the matching
 * halfword of the 152-byte work-record block at 0x80147A58 (offsets 0x2E and
 * 0x30) plus the matching signed byte of the two-byte cursor offset pair at
 * 0x801FC9E8 (0x0C for x, 0xC8 for y), clears the object's countdown byte at
 * 0x09, sets its primitive index byte at 0x06 to 0x20, sets the three tint
 * channels at offsets 0x5F, 0x5E and 0x5D to -128 (0x80), runs the
 * overlay-local sprite emitter func_801F8BCC, and finally copies the object's
 * byte at 0x07 into its dispatch byte at 0x01. Takes no arguments and returns
 * nothing; the address is entry 21 of the per-frame handler table at
 * 0x801FC980 and the first word of the jump table at 0x801FC9D4 that the
 * overlay's secondary dispatcher at 0x801F8748 indexes with that dispatch
 * byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F878C(void) {
  u8 *work;
  u8 *flags;
  u8 *tint;
  u8 *state;

  work = D_1F800044;
  *(u16 *)(work + 0x2E) = *(u16 *)(D_80147A58 + 0x2E) + (s8)D_801FC9E8[0];
  *(u16 *)(work + 0x30) = *(u16 *)(D_80147A58 + 0x30) + (s8)D_801FC9E8[1];
  *(u8 *)(work + 0x09) = 0;
  flags = D_1F800044;
  *(u8 *)(flags + 0x06) = 0x20;
  tint = D_1F800044;
  *(s8 *)(tint + 0x5F) = -128;
  *(s8 *)(tint + 0x5E) = -128;
  *(s8 *)(tint + 0x5D) = -128;
  func_801F8BCC();
  state = D_1F800044;
  *(u8 *)(state + 0x01) = state[0x07];
}
