#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2C88
 * @behavior Queues the four shared UI elements of one two-by-two block through
 *           the shared marker queue func_801A4BC0: the two cells at y = 0xB of
 *           x = 0x48 and 0x49 with size 0, then the two cells at y = 0xC of the
 *           same two columns with size 0. Takes no arguments and returns
 *           nothing, and its 0x18-byte frame only keeps $ra across the four
 *           calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the four jal instructions at 0x801F2C98, 0x801F2CA8,
 * 0x801F2CB8 and 0x801F2CC8 all target the shared marker queue func_801A4BC0,
 * and the two words before each of them carry this function's own literals
 * from the original instruction words - a0 = 0x48, 0x49, 0x48, 0x49 with a1 =
 * 0xB, 0xB, 0xC, 0xC and a2 = 0 in all four delay slots (`addu a2,zero,zero`).
 * That is two adjacent x columns over two rows at a single size, so the name
 * follows the block grammar of
 * queueSharedUiElementBlockFTo10x3BTo3CSize0And51World04Area18813_801F494C and
 * queueSharedUiElementBlock4DTo4Ex22To25Size0World01Area04914_801F37EC.
 *
 * Shape notes from the original bytes: the span is code rather than data or a
 * shared epilogue, because it opens with its own `addiu sp,sp,-0x18` frame at
 * 0x801F2C88, ends with its own `jr $ra` at 0x801F2CD8 and the next function
 * starts immediately at 0x801F2CE0. A scan of every payload under
 * out/binaries for these 0x58 bytes finds only this copy, so the clone lever
 * does not apply. The nearest exact sibling,
 * queueSharedUiElementBlockFTo10x3BTo3CSize0And51World04Area18813_801F494C at
 * 0x801F494C, carries the same call topology (two x columns visited per row,
 * two rows) with different literals, so it supplied the statement order,
 * operand forms and declaration style while every literal here is written from
 * this span's own bytes. The pointer run at 0x801F36A0 lists this address in
 * its third word (0x801F36A8) along with the six other handlers of this
 * overlay, so the entry is a script-invoked state handler and its argument
 * roles stay structural. The neighbouring entry at 0x801F2CE0 queues the same
 * two-by-two block with sizes 0xC0 and 0xA1 and is not part of this lift.
 */
void queueSharedUiElementBlock48To49xBToCSize0World00Area02313_801F2C88(void) {
  func_801A4BC0(0x48, 0xb, 0);
  func_801A4BC0(0x49, 0xb, 0);
  func_801A4BC0(0x48, 0xc, 0);
  func_801A4BC0(0x49, 0xc, 0);
}
