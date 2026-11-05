#include "bof3/bof3.h"

/* Same scratchpad pointer-cell view as this target's exact motion initializer. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Original callee returns a table-selected byte and consumes no arguments. */
u8 func_800AFF98(void);

/* @source 0x801F133C
 * @behavior Calls the helper at 0x800AFF98 with no arguments, stores its returned
 * byte in scratchpad work byte 9, then reloads the work pointer at 0x1F800044
 * and increments work byte 2. The helper selects a lookup byte for the current
 * battle participant. No other fields are accessed by this handler.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic00800_801F133C(void) {
  u8 value;

  value = func_800AFF98();
  g_battle_work[9] = value;
  g_battle_work[2]++;
}
