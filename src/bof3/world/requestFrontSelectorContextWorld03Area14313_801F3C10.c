#include "bof3/bof3.h"

/* Shared front selector context request (selection seed, context pair, kind),
 * declared identically by the sibling world targets that call it. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F3C10
 * @behavior Requests the shared front selector context through func_8019FA28
 * with the fixed tuple: selection seed 0x70, context pair 0xC8000/0x580000 and
 * kind 0x81. It is the first handler pointer of the area's action table at
 * 0x801F52A8. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestFrontSelectorContextWorld03Area14313_801F3C10(void) {
  func_8019FA28(0x70, 0xC8000, 0x580000, 0x81);
}
