#include "bof3/bof3.h"

/* Shared front selector context request (selection seed, context pair, kind),
 * declared identically by the sibling world targets that call it. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F2C04
 * @behavior Entry 0 of the three-entry handler run at 0x801F3AB0, which the
 *           overlay names through the pointer word at 0x801F3AF8 and whose word
 *           1 is 0x801F2C34 and word 2 the record-flag clear at 0x801F2C68:
 *           requests the shared front selector context through func_8019FA28
 *           with the fixed tuple selection seed 0x70, context pair
 *           0xC8000/0x580000 and kind 0x81. It takes no arguments, returns
 *           nothing and reads no state of its own.
 * Shape evidence: the byte-identical lifts
 * requestFrontSelectorContextWorld03Area14313_801F3C10 and
 * requestFrontSelectorContextWorld03Area14514_801F3FD8 carry this body, and a
 * payload cmp of this span at 0x801F2C04 against theirs at 0x801F3C10 of
 * WORLD03/AREA143 and 0x801F3FD8 of WORLD03/AREA145 measured all 48 bytes
 * equal, so the statement order, operand forms and declaration style come from
 * that pair and the handler-run position supplies the name.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestFrontSelectorContextWorld03Area14613_801F2C04(void) {
  func_8019FA28(0x70, 0xC8000, 0x580000, 0x81);
}
