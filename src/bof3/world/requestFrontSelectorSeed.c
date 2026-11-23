#include "bof3/bof3.h"

/* Shared front selector context request (selection seed, context pair, kind),
 * declared identically by the sibling world targets that call it. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F2C04
 * @behavior Requests the shared front selector context through func_8019FA28
 * with the fixed tuple: selection seed 0x64, context pair 0x460000/0x340000 and
 * kind 0x81. Nothing in the payload jumps here - the address appears only as
 * the data word 0x801F32B8, an entry of the pointer run that starts at
 * 0x801F3298 inside this overlay's blob - so the entry takes no arguments and
 * returns nothing.
 *
 * Shape evidence: the same body sits at payload offset 0x130 of
 * BIN/WORLD01/AREA059#13 (0x801F2D30) and at offset 0x154 of
 * BIN/WORLD03/AREA116#13 (0x801F2D54), and a byte comparison of the three
 * forty-four-byte spans compared equal word for word, the absolute jal
 * immediate included. Both sibling rows are still unlifted asm spans, so the
 * clone lever had no source to copy from and this body is seeded from this
 * payload's own bytes. Statement order, operand forms and declaration style are
 * transcribed from the exact sibling
 * requestFrontSelectorContextWorld03Area14313_801F3C10, whose body differs from
 * this one in the four literals it passes alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestFrontSelectorSeed(void) {
  func_8019FA28(0x64, 0x460000, 0x340000, 0x81);
}
