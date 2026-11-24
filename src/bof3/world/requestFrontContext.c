#include "bof3/bof3.h"

/* Shared front selector context request (selection seed, context pair, kind),
 * declared identically by the sibling world targets that call it. */
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);

/* @source 0x801F2D30
 * @behavior Requests the shared front selector context through func_8019FA28
 * with the fixed tuple: selection seed 0x64, context pair 0x460000/0x340000 and
 * kind 0x81. Nothing in the payload jumps here - no jal names the address, and
 * it appears only as the data word at 0x801F37A4 in this overlay's tail pointer
 * blob, beside the words 0x801F2D5C, 0x801F2D90 and 0x801F2E08 - so the entry
 * takes no arguments and returns nothing.
 *
 * Shape evidence: this forty-four-byte span compared equal word for word, the
 * absolute jal immediate included, to the span at 0x801F2C04 of the sibling
 * overlay BIN/WORLD00/AREA036.EMI#13 and to the unlifted asm span at 0x801F2D54
 * of BIN/WORLD03/AREA116.EMI#13, and the clone dry run against the sibling's
 * lift requestFrontSelectorContextWorld00Area03613_801F2C04 rewrote no
 * identifier. Statement order, operand forms and declaration style are
 * transcribed from that sibling.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestFrontContext(void) {
  func_8019FA28(0x64, 0x460000, 0x340000, 0x81);
}
