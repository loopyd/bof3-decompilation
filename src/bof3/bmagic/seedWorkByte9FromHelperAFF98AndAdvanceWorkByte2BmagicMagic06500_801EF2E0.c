#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, shared by every overlay and
 * bound by this target's map under the same name the exact bmagic siblings use.
 * The volatile pointer keeps the original's reload of the cell before each
 * access. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Out-of-payload callee already bound by this target's generated function map;
 * returns a table-selected byte and consumes no arguments. */
u8 func_800AFF98(void);

/* @source 0x801EF2E0
 * @behavior Entry 0 of the four-entry code pointer table at 0x801EEC14
 *           (0x801EF2E0, 0x801EF328, 0x801EF394, 0x801EF408), which 0x801EF250
 *           copies into its own frame and dispatches on the scratchpad work
 *           object's state byte 2. Calls the helper at 0x800AFF98 with no
 *           arguments, stores its returned byte in work byte 9, then reloads
 *           the pointer cell at 0x1F800044 and increments work byte 2 by one at
 *           byte width, which selects table entry 1. The helper selects a lookup
 *           byte for the current battle participant; no other field is read or
 *           written, nothing is returned, and the 0x18-byte frame only keeps
 *           $ra across the call.
 *
 *           Bytes 0x801EF2E0..0x801EF327 are byte-identical to the exact lift
 *           0x801F133C of bmagic/magic008/00 and to 0x801EF9A0 of
 *           bmagic/magic004/03 (two 72-byte payload cmps), whose body is cloned
 *           here; both twins encode the same scratchpad cell and the same
 *           absolute callee, so no word shifts between the spans.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic06500_801EF2E0(void) {
  u8 value;

  value = func_800AFF98();
  g_battle_work[9] = value;
  g_battle_work[2]++;
}
