#include "bof3/bof3.h"

void func_801A4BC0(s32 x, s32 y, u32 size);

/* @source 0x801F5DD0
 * @behavior Queues the four shared UI elements of the horizontal run
 *           x = 4..7 at y = 5 through the shared marker queue func_801A4BC0
 *           with size 0x50, one element per iteration of a counter bounded by
 *           4. The counter is an unsigned byte local: the `andi $v0,$v0,0xFF`
 *           after `addiu $s0,$s0,1` truncates the increment, the bound is the
 *           unsigned `sltiu $v0,$v0,4`, and the x argument is that masked
 *           counter plus 4, with the mask scheduled into the branch delay slot
 *           of the loop test and the same mask in the loop preheader priming
 *           $a0 for the first iteration. Takes no arguments and returns
 *           nothing; the 0x18-byte frame only keeps $s0 and $ra across the
 *           calls.
 *
 *           Naming evidence: the span holds one `jal`, at 0x801F5DEC, to the
 *           shared marker queue func_801A4BC0, and its delay slot at
 *           0x801F5DF0 carries this function's own size literal 0x50, while
 *           0x801F5DE8 sets the y argument to 5 and 0x801F5DE4 adds 4 to the
 *           masked counter. One callee, one y, one size and four consecutive
 *           x values make this the block grammar used by the sibling
 *           queueSharedUiElementBlock94To97x20To23Size0World01Area04013_801F369C,
 *           which reaches the same callee from its own bases and size.
 *
 *           Call evidence: the only word in the payload naming this address is
 *           the pointer cell at 0x801F650C inside the trailing initialized data
 *           run that begins at 0x801F64F8, and no `jal` immediate names it, so
 *           the caller is a dispatched callback whose blob carries no
 *           prototype; the parameter roles stay structural.
 * @status exact
 * @match 100.00
 * @residual none
 */
void queueSharedUiElement(void) {
  u8 i;

  for (i = 0; i < 4; i++) {
    func_801A4BC0(i + 4, 5, 0x50);
  }
}
