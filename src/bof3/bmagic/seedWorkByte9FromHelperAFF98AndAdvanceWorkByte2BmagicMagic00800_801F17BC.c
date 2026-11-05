#include "bof3/bof3.h"

/* Same scratchpad pointer-cell view as this target's exact motion initializer. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Original callee returns a table-selected byte and consumes no arguments. */
u8 func_800AFF98(void);

/* @source 0x801F17BC
 * @behavior Entry 0 of the nine-word handler table at 0x801EECC8 that
 * 0x801F1708 copies to its frame and dispatches on scratchpad work byte 2.
 * Calls the helper at 0x800AFF98 with no arguments, stores its returned byte in
 * work byte 9, then reloads the work pointer at 0x1F800044 and increments work
 * byte 2. The bytes are identical to the overlay's other copy at 0x801F133C, so
 * the handler body and its only callee are the same ones recorded there.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic00800_801F17BC(void) {
  u8 value;

  value = func_800AFF98();
  g_battle_work[9] = value;
  g_battle_work[2]++;
}
