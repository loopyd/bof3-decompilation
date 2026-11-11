#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, shared by every overlay.
 * The volatile pointer keeps the original's reload of the cell before each
 * access. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Out-of-payload callee already bound by this target's generated function map;
 * returns a table-selected byte and consumes no arguments. */
u8 func_800AFF98(void);

/* @source 0x801EF9A0
 * @behavior Calls the helper at 0x800AFF98 with no arguments, stores its
 *           returned byte in byte 9 of the scratchpad work object read through
 *           the pointer cell at 0x1F800044, then reloads that cell and
 *           increments work byte 2 by one at byte width. The helper selects a
 *           lookup byte for the current battle participant; no other field is
 *           read or written, nothing is returned, and the 0x18-byte frame only
 *           keeps $ra across the call. Bytes 0x801EF9A0..0x801EF9E7 are
 *           byte-identical to the exact lift 0x801F133C of bmagic/magic008/00
 *           (one 72-byte payload cmp), whose body is cloned here with only the
 *           helper's spelling and the work cell's name changed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic00403_801EF9A0(void) {
  u8 value;

  value = func_800AFF98();
  g_battle_work[9] = value;
  g_battle_work[2]++;
}
