#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F35D4
 * @behavior Queues the two shared UI elements of one row through the shared
 *           marker queue func_801A4BC0: the element at x = 0xB, y = 0x55 with
 *           size 0, then the element at x = 0xC on the same row with size 0.
 *           Takes no arguments and returns nothing, and its 0x18-byte frame
 *           only keeps $ra across the two calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the two jal instructions at 0x801F35E4 and 0x801F35F4 both
 * target the shared marker queue func_801A4BC0, and the two words before each
 * of them carry this span's own literals from the original instruction words -
 * a0 = 0xB then 0xC with a1 = 0x55 in both, and a2 = 0 in both delay slots
 * (`addu a2,zero,zero`). That is two adjacent x columns at a single row, so the
 * name follows the block grammar of
 * queueSharedUiElementBlock48To49xBToCSize0World00Area02313_801F2C88 and
 * queueSharedUiElementBlock4DTo4Ex22To25Size0World01Area04914_801F37EC. The
 * element pair is the one its neighbour at 0x801F360C later queues with size
 * 0xC0, and this entry queues it with size 0.
 *
 * Shape notes from the original bytes: the span is code rather than data or a
 * shared epilogue, because it opens with its own `addiu sp,sp,-0x18` frame at
 * 0x801F35D4, ends with its own `jr $ra` at 0x801F3604 and the next function
 * starts immediately at 0x801F360C. No payload under out/binaries holds a
 * second copy of these 0x38 bytes, so no byte-identical sibling exists and the
 * clone lever does not apply. The nearest exact sibling,
 * queueSharedUiElementBlock4DTo4Ex22To25Size0World01Area04914_801F37EC,
 * carries the same call topology (constant x/y literals with a zero size) with
 * different literals, so it supplied the statement order, operand forms and
 * declaration style while every literal here is written from this span's own
 * bytes. This payload declares no pointer table over this span and no other
 * span calls it, so the argument roles stay structural.
 */
void queueSharedUiElementBlockBToCx55Size0World01Area04914_801F35D4(void) {
  func_801A4BC0(0xb, 0x55, 0);
  func_801A4BC0(0xc, 0x55, 0);
}
