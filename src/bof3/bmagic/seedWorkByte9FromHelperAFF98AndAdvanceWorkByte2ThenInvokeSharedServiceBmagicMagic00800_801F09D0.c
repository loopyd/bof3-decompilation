#include "bof3/bof3.h"

/* Same scratchpad pointer-cell view as this target's exact motion initializer. */
extern u8* volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Original callee returns a table-selected byte and consumes no arguments. */
u8 func_800AFF98(void);

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header; this caller passes no
 * arguments. */
void func_8014D290(void);

/* @source 0x801F09D0
 * @behavior Entry 0 of the four-entry work-state table at 0x801EEC84
 * (0x801F09D0, 0x801F0A20, 0x801F0AC8, 0x801F0B44), which the dispatcher at
 * 0x801F0964 copies into its frame and selects with scratchpad work-object
 * byte 2. Calls the helper at 0x800AFF98 with no arguments, stores its
 * returned byte in scratchpad work byte 9, reloads the work pointer cell at
 * 0x1F800044 and increments work byte 2 with a byte-width read-modify-write,
 * moving dispatch to entry 1 at 0x801F0A20, then invokes the shared
 * main-executable service at 0x8014D290 unconditionally, with the byte-2 store
 * filling that call's delay slot. The pointer cell is read once per access, so
 * the helper's returned byte is written through the first read and the
 * incremented byte 2 through the second. The 0x18-byte frame keeps only $ra at
 * 0x10 across the two calls; it takes no arguments, returns nothing and reads
 * or writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2ThenInvokeSharedServiceBmagicMagic00800_801F09D0(
    void) {
  u8 value;

  value = func_800AFF98();
  g_battle_work[9] = value;
  g_battle_work[2]++;
  func_8014D290();
}
