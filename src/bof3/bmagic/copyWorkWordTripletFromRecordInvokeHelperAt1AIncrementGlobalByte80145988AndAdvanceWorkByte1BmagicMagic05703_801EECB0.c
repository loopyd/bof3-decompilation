#include "bof3/bof3.h"

/* Shared battle work pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt
 * row D_801EB4E0 = 0x801EB4E0, so no target-map row is needed. The body reads
 * its words 0x34, 0x38 and 0x3C, the same three-word group the exact sibling
 * copyWorkTripletSeedOffsetPairAndAdvanceWorkByte2BmagicMagic10503_801EF06C
 * copies in its own target. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Scratchpad work-object pointer cell shared by the battle and bmagic
 * overlays; this target's map binds it at 0x1F800044 as g_battle_work. The
 * original materialises the cell with its own lui 0x1F80 / lw 0x44 pair at
 * each of the two groups, the second pair being a fresh load after the call
 * rather than a reuse of the first. The cell is read once per group and serves
 * both the load and the store of that group, so the declaration keeps the byte
 * view only. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* One-argument overlay helper whose callers in this tree pass encoded engine
 * action ids; this body passes 0x1A. Bound by this target's generated
 * undefined_funcs_auto.txt row func_8014E3A0 = 0x8014E3A0, so no target-map
 * row is needed and the callee lives outside this payload. */
extern void func_8014E3A0(u32 action); /* @source 0x8014E3A0 */

/* Shared main-RAM byte outside this overlay's image, bound by this target's
 * generated undefined_syms_auto.txt row D_80145988 = 0x80145988. Other lifted
 * targets in this tree describe it as a stage serial, but nothing in this
 * target establishes that role, so the name keeps the address anchor. */
extern u8 D_80145988; /* @source 0x80145988 @kind data */

/* @source 0x801EECB0
 * @behavior First handler of this overlay's three-word handler table at
 *           0x801EEC04 (0x801EECB0, 0x801EED2C, 0x801EEFEC), whose dispatcher
 *           func_801EEC50 copies those three words into its own 0x28-byte
 *           frame and indexes them with the scratchpad work object's byte
 *           0x01; no jal in the image names this span.
 *           It reads the shared record pointer at 0x801EB4E0 and the scratchpad
 *           work-object pointer cell at 0x1F800044 once each, then copies the
 *           record words 0x34, 0x38 and 0x3C to the same three word offsets of
 *           the work object. Each copy is a bare load and store pair, with the
 *           load latency filled by a nop because the next load cannot be hoisted
 *           above a store through the other pointer.
 *           It then requests the engine action 0x1A through func_8014E3A0,
 *           whose return value it ignores, and afterwards re-reads the
 *           work-object cell rather than reusing the pre-call pointer.
 *           Last it increments the shared main-RAM byte at 0x80145988 and
 *           increments work byte 0x01, whose value selects an entry of the
 *           handler table above, so this body hands the next frame of that
 *           sequence to the handler at 0x801EED2C. Both increments are
 *           byte-wide and neither value is tested. The word 0x34 filler path
 *           stays offset-anchored: this body only reads that record group,
 *           while the sibling handlers rewrite it.
 *           It reads or writes no other state, takes no arguments and returns
 *           nothing through the jr $ra at 0x801EED24 inside a 0x18-byte frame
 *           that keeps $ra at 0x10.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copyWorkWordTripletFromRecordInvokeHelperAt1AIncrementGlobalByte80145988AndAdvanceWorkByte1BmagicMagic05703_801EECB0(void) {
  u32 *source;
  u32 *work;

  source = (u32 *)D_801EB4E0;
  work = (u32 *)g_battle_work;
  work[13] = source[13];
  work[14] = source[14];
  work[15] = source[15];
  func_8014E3A0(0x1A);
  D_80145988 = D_80145988 + 1;
  g_battle_work[1] = g_battle_work[1] + 1;
}
