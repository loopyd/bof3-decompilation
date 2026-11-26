#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. Spelled as the named fixed address
 * rather than through the constant-address accessor because the original
 * re-emits `lui $vN,0x1F80` plus the folded `lw $vN,68($vN)` for the byte +2
 * increment after the flag tester returns and the byte +9 store in between.
 * The `D_<ADDRESS>` spelling binds with no map row.
 * @source 0x1F800044 @kind unknown
 */
extern u8* D_1F800044;

/* Main-RAM flag-bank word the gate reads. The body materialises it with
 * `lui $a0,0x8014` / `addiu $a0,$a0,-0x1108` and passes it with the bit index
 * 0x0E, so it is an unsigned byte cell of the shared bank.
 * @source 0x80144EF8 @kind data
 */
extern u8 D_80144EF8;

/* Shared flag tester of the main executable, outside this payload. The caller
 * passes the flag-bank word and the bit index and a non-zero answer reports the
 * flag as set; this target's generated undefined_funcs_auto.txt binds the
 * address, so the raw `func_` name stays and no map row is owed. */
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* Overlay-local routine at 0x801F3768, still an `asm` row of this target. The
 * sibling handler 0x801F36F0 passes `0x80 - 8 * byte9` in its first argument
 * and `8 * byte9` in its second, so the first argument is the quantity this
 * function passes at full value 0x80 and the second is the paired offset it
 * passes as zero. */
void renderBandColumns(u32 arg0, u32 arg1); /* @source 0x801F3768 */

/* @source 0x801F368C
 * @behavior State-byte-2 handler of this overlay: word 1 of the payload's
 *           handler table 0x801F4368 holds this address, so no `jal` immediate
 *           names it. Every frame it calls the overlay-local routine 0x801F3768
 *           with the full first argument 0x80 and the second argument 0. When
 *           bit 0x0E of the main-RAM flag-bank word 0x80144EF8 is clear through
 *           the shared flag tester 0x8015B5D4 the body returns after that call
 *           and the record stays on this handler. When the flag tester reports
 *           the bit set the body also clears the work-record byte at +9 and
 *           increments the dispatch byte at +2, so the byte-2 dispatch run moves
 *           to word 2 of the same table, the handler 0x801F36F0. That handler
 *           advances byte +9 toward 0x10 and shrinks the first argument by 8 per
 *           step, so this handler is the arm that reopens the full value and
 *           restarts the step count. The two accesses re-read the work-record
 *           pointer cell 0x1F800044 separately, and the 0x18-byte frame holds
 *           only ra at +0x10. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitDrawingFullWidth(
    void) {
  if (func_8015B5D4((u32)&D_80144EF8, 0x0E) != 0) {
    D_1F800044[9] = 0;
    D_1F800044[2]++;
  }
  renderBandColumns(0x80, 0);
}
