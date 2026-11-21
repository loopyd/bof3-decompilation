#include "bof3/bof3.h"

/* Shared front selector context request (selection seed, context pair, kind),
 * declared identically by the sibling world targets that call it. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F2D54
 * @behavior Requests the shared front selector context through func_8019FA28
 * with the fixed tuple: selection seed 0x64, context pair 0x460000/0x340000 and
 * kind 0x81. Nothing in this payload jumps here - no jal encodes the address -
 * and the word 0x801F2D54 occurs only once, at 0x801F3478: the ninth entry of
 * the code-pointer run that begins at 0x801F3458 inside this overlay tail blob,
 * immediately before the word 0x801F2D80, and itself stored again at 0x801F34AC
 * and 0x801F34E0 - so the entry takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestFrontSelectorContextWorld03Area11613_801F2D54(void) {
  func_8019FA28(0x64, 0x460000, 0x340000, 0x81);
}
