#include "bof3/bof3.h"

/* Shared front selector context request (selection seed, context pair, kind),
 * declared identically by the sibling world targets that call it. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F3FD8
 * @behavior Requests the shared front selector context through func_8019FA28
 * with the fixed tuple: selection seed 0x70, context pair 0xC8000/0x580000 and
 * kind 0x81. The overlay code-pointer run at 0x801F5C20 holds this address at
 * the word 0x801F5C2C. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestFrontSelectorContextWorld03Area14514_801F3FD8(void) {
  func_8019FA28(0x70, 0xC8000, 0x580000, 0x81);
}
