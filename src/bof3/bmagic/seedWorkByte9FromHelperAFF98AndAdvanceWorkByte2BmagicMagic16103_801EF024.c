#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's handlers. The
 * original materialises the cell address (lui $v1,0x1F80 / lw $v1,68($v1))
 * twice, once after the helper call and once for the work-byte-2 increment,
 * because the call may publish a new work object. */
extern u8 *volatile g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable helper at 0x800AFF98; it returns the lookup byte this
 * handler stores in work byte 9 and consumes no arguments. */
u8 func_800AFF98(void);

/* @source 0x801EF024
 * @behavior Work byte 2 step handler of this overlay: it calls the shared helper
 * at 0x800AFF98 with no arguments, stores the returned byte in scratchpad work
 * byte 9 through the pointer cell at 0x1F800044, then re-reads that cell and
 * increments work byte 2 at byte width, advancing the dispatch state. The helper
 * selects a lookup byte for the current battle participant. The 0x18-byte frame
 * only keeps the return address at 0x10; no other state is read or written and
 * nothing is returned. The 72-byte body is byte-identical to the bodies at
 * 0x801F133C of emi/bmagic/magic008/00, 0x801EF9A0 of emi/bmagic/magic004/03
 * and 0x801EF2E0 of emi/bmagic/magic065/00 (payload byte comparison), and those
 * sources measured exact when this seed was transcribed from the magic008/00
 * body.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkByte9FromHelperAFF98AndAdvanceWorkByte2BmagicMagic16103_801EF024(void) {
  u8 value;

  value = func_800AFF98();
  g_battle_work[9] = value;
  g_battle_work[2]++;
}
