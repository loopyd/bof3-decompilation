#include "bof3/bof3.h"

u32 func_801F49A0(void);
void func_80196070(void);

/* @source 0x801F4310
 * @behavior Overlay conditional work-flag reset: calls the table-walk helper
 * at 0x801F49A0, which scans the 0x20 entries of the 0x28-byte-stride work
 * table at 0x800E4800, runs each selected entry's handler and leaves 1 in its
 * return register once at least one entry was processed (0 otherwise); when
 * the low byte of that result is zero (no callback ran) it calls the shared
 * work-area reset helper at 0x80196070, the address mapped as
 * clearWorkFlags = 0x80196070 in emi/etc/game/00. It takes no arguments,
 * returns nothing, reads no state of its own and writes none beyond what the
 * two callees touch; the 0x18-byte frame only keeps $ra across the calls and
 * the second call is reached only on the zero path. The callee prototype below
 * is the caller-side width: only the low byte of the result is consumed. Its
 * 0x34 bytes are the same shape as the exact sibling at 0x801F6F14 in the
 * scena07 overlay.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkFlagsWhenNoCallbackRanWorld03Area13413_801F4310(void) {
  if ((func_801F49A0() & 0xFF) == 0) {
    func_80196070();
  }
}
