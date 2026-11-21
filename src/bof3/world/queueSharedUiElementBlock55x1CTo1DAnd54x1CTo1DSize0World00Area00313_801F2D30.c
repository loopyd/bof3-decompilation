#include "bof3/bof3.h"

/* Shared world-marker queue: the sibling world targets declare the same
 * prototype (it is absent from the shipped headers), and this payload's
 * generated undefined_funcs_auto.txt row binds the address, so the name keeps
 * the raw spelling.
 * @source 0x801A4BC0 @kind function */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2D30
 * @behavior Queues four shared UI elements through func_801A4BC0: the element
 *           at (0x55, 0x1C) with size 0, the element at (0x55, 0x1D) with size
 *           0, the element at (0x54, 0x1C) with size 0 and the element at
 *           (0x54, 0x1D) with size 0. Takes no arguments and returns nothing;
 *           its 0x18-byte frame only keeps $ra across the four calls, and $v0
 *           carries the zero third argument through each call's delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the four jal words at 0x801F2D40, 0x801F2D50, 0x801F2D60 and
 * 0x801F2D70 all target the shared marker queue func_801A4BC0, and their delay
 * slots carry this function's own literals from the original instruction words
 * - a0 = 0x55, 0x55, 0x54, 0x54, a1 = 0x1C, 0x1D, 0x1C, 0x1D and a2 = 0 on
 * every call. That is a two-column block, x = 0x55 covering y = 0x1C to 0x1D
 * and x = 0x54 covering the same rows, emitted column by column, so the name
 * follows the block grammar of the
 * queueSharedUiElementBlock11x15To16And12x16Size0World00Area03313_801F2CD0 and
 * queueSharedUiElementBlockCx1EAnd1CTo1DSizeC0World01Area03813_801F2C4C
 * siblings with the coordinates in call order and the shared size last.
 *
 * Shape evidence: the body has its own prologue, its four calls and its own
 * jr $ra, so it is code rather than data, and no jal word in this payload
 * targets it, so it is an overlay-loader or script entry with structural
 * argument roles. The 0x58-byte span is byte-identical to the four-call body at
 * 0x801F461C of World00 Area008 EMI#13: one payload cmp of the two 88-byte
 * spans (payload offsets 304 and 6684) reports no differing byte, every
 * absolute word included, because both spans reach the same shared callee.
 * That twin is already lifted exact, so its statement order, operand forms and
 * declaration style seeded this body, while the constants above come from this
 * span's own words. Its include is a target-local area00813_internal.h, which
 * lift clone refuses to rewrite, so the shared callee declaration is spelled
 * locally as the queueSharedUiElementBlock11x15To16And12x16Size0World00Area03313
 * _801F2CD0 sibling also does.
 */
void queueSharedUiElementBlock55x1CTo1DAnd54x1CTo1DSize0World00Area00313_801F2D30(void) {
  func_801A4BC0(0x55, 0x1C, 0);
  func_801A4BC0(0x55, 0x1D, 0);
  func_801A4BC0(0x54, 0x1C, 0);
  func_801A4BC0(0x54, 0x1D, 0);
}
