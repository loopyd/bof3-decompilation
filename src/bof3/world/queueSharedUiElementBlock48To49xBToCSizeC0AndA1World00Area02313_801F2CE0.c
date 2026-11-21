#include "bof3/bof3.h"

void func_801A4BC0(s16 x, s16 y, u32 size);

/* @source 0x801F2CE0
 * @behavior Queues the four shared UI elements of one two-by-two block through
 *           the shared marker queue func_801A4BC0: the two cells at y = 0xB of
 *           x = 0x48 and 0x49 with size 0xC0, then the two cells at y = 0xC of
 *           the same two columns with size 0xA1. Takes no arguments and returns
 *           nothing, and its 0x18-byte frame only keeps $ra across the four
 *           calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Naming evidence: the four jal instructions at 0x801F2CF0, 0x801F2D00,
 * 0x801F2D10 and 0x801F2D20 all target the shared marker queue func_801A4BC0,
 * and their delay slots carry this function's own literals from the original
 * instruction words - a0 = 0x48, 0x49, 0x48, 0x49 with a1 = 0xB, 0xB, 0xC,
 * 0xC and a2 = 0xC0, 0xC0, 0xA1, 0xA1, each spelled `addiu a2,zero,imm`. That
 * is two adjacent x columns over two rows with one size per row, so the name
 * follows the block grammar of the neighbouring
 * queueSharedUiElementBlock48To49xBToCSize0World00Area02313_801F2C88 and the
 * size-pair form of
 * queueSharedUiElementBlock1CTo1Dx3SizeC0And20x15To16SizeA1World04Area18813_801F4A54,
 * with both sizes in body order.
 *
 * Shape notes from the original bytes: the span is code rather than data or a
 * shared epilogue, because it opens with its own `addiu sp,sp,-0x18` frame at
 * 0x801F2CE0, ends with its own `jr $ra` at 0x801F2D30 and the next declared
 * row starts immediately at 0x801F2D38, so its 0x58 bytes are claimed whole
 * with no slack. A byte scan of the 398 payloads under out/binaries for these
 * 0x58 bytes finds only this span, at payload offset 0xE0 of
 * emi/world00/area023/13.bin, so the clone lever does not apply. The nearest
 * exact sibling,
 * queueSharedUiElementBlock48To49xBToCSize0World00Area02313_801F2C88, differs
 * from this span in exactly four words - its delay slots spell
 * `addu a2,zero,zero` where this body stores the immediates 0xC0 and 0xA1 -
 * so it supplied the statement order, operand forms and declaration style
 * while every literal here is written from this span's own bytes. The pointer
 * run at 0x801F36A0 lists this address in its fourth word (0x801F36AC) along
 * with the six other handlers of this overlay, so the entry is a
 * script-invoked state handler and its argument roles stay structural.
 */
void queueSharedUiElementBlock48To49xBToCSizeC0AndA1World00Area02313_801F2CE0(
    void) {
  func_801A4BC0(0x48, 0xb, 0xc0);
  func_801A4BC0(0x49, 0xb, 0xc0);
  func_801A4BC0(0x48, 0xc, 0xa1);
  func_801A4BC0(0x49, 0xc, 0xa1);
}
