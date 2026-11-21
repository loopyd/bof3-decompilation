#include "bof3/bof3.h"

/* @source 0x801A4BC0 @kind function */
extern void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2CC4
 * @behavior Queues the four shared UI elements of the two-by-two block
 *           x = 0x03..0x04, y = 0x07..0x08 through the shared world-marker
 *           queue 0x801A4BC0, every one with size 0: the two elements at
 *           y = 0x07 and 0x08 of x = 0x03 followed by the two elements at
 *           y = 0x07 and 0x08 of x = 0x04. Takes no arguments and returns
 *           nothing; its 0x18-byte frame only keeps the return address across
 *           the four calls and closes on `jr $ra` at 0x801F2D14 with its
 *           delay `nop`.
 *
 *           Evidence from this span's own bytes: the four coordinate pairs are
 *           the `addiu` immediates at span offsets 0x08/0x0C, 0x18/0x1C,
 *           0x28/0x2C and 0x38/0x3C, the size argument is the `addu $a2,$zero,
 *           $zero` in each call's delay slot, and every `jal` targets the
 *           shared main-executable callee 0x801A4BC0, so none of these values
 *           depends on this overlay's own data.
 *
 *           Shape: the four-call body of func_801F461C in the sibling target
 *           emi/world00/area008/13 (measured exact in this tree at the time of
 *           this lift) has the same statement order, operand forms and unrolled
 *           literal-call spelling. A word comparison of the two 0x58-byte
 *           spans (payload offsets 0xC4 and 0x1A1C, both overlays loading at
 *           0x801F2C00) differs in exactly the eight coordinate immediate words
 *           and in nothing else, so it is shape evidence and not a byte
 *           identity: no clone was claimed. Three further carriers of the same
 *           body with other literals sit at 0x801F2C54 (world00/area013/13),
 *           0x801F2C88 (world00/area023/13) and 0x801F2D30 (world00/area003/13),
 *           all three still on their `asm` rows; each was compared the same way
 *           and differs from this span in those eight immediate words alone.
 *
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSharedUiElementBlock3To4x7To8Size0World00Area02213_801F2CC4(void) {
  func_801A4BC0(0x03, 0x07, 0);
  func_801A4BC0(0x03, 0x08, 0);
  func_801A4BC0(0x04, 0x07, 0);
  func_801A4BC0(0x04, 0x08, 0);
}
