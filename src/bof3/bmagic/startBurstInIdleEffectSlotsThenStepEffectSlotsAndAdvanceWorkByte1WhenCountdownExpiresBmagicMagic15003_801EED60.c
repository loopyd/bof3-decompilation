#include "bof3/bof3.h"

/* Burst seed of this overlay at 0x801EF6F8, still an asm row in this target's
 * Splat listing and therefore already defined by the link. Its own bytes take no
 * arguments and return nothing: it points the slot cursor 0x801EFAF4 back at the
 * base of the 8-record effect-slot run 0x801EFAB4 and, for every record whose
 * offset-0 halfword is zero (idle), writes 1 into that halfword, 0x300 into the
 * offset-2 halfword, (rand() & 0xFF) << 4 into the offset-4 halfword and 8 into
 * the offset-6 halfword. One call therefore starts an eight-step burst in every
 * idle slot, with radius scale 0x300 and a random angle. */
extern void func_801EF6F8(void); /* @source 0x801EF6F8 */

/* Slot step routine at 0x801EF8EC, still an asm row in this target's Splat
 * listing. It walks the same 8-record run 0x801EFAB4, and for each active record
 * rotates the offset-4 angle and the offset-2 radius scale into a particle
 * position, draws it with the fan at 0x801EF790, advances the angle by 0x80 and
 * the radius scale by -0x60, clears the offset-0 halfword when that scale reaches
 * zero, and returns 1 in $v0 when any record was active. This span ignores that
 * answer, so the call's return value is unused here. */
extern u32 func_801EF8EC(void); /* @source 0x801EF8EC */

/* Scratchpad work-object pointer cell 0x1F800044, shared by all overlays and
 * bound as g_battle_work by this target's own map row and spelled the same way
 * by this target's existing exact lifts. The original re-materializes the cell
 * base for each access (lui $vN,0x1F80 / lw $vN,68($vN) before the byte-9 store
 * and again before the byte-1 update), so the cell stays the named symbol rather
 * than a pointer local that would fold the second read into the first. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801EED60
 * @behavior Magic-150 effect handler, the entry at 0x801EEC08 of the overlay
 *           handler table that starts at 0x801EEC04, so index 1 of its seven
 *           words; the dispatcher 0x801EEC20 indexes that table by work-object
 *           byte 1. The body creates a 0x18-byte frame with $ra at +0x10 and
 *           calls 0x801EF6F8 first, which starts an eight-step burst in every
 *           idle slot of the effect-slot run 0x801EFAB4, then calls 0x801EF8EC
 *           with no arguments, which steps and draws the active slots of that
 *           run; both answers are discarded. It then reads the work-object
 *           pointer from scratchpad cell 0x1F800044, decrements the work-object
 *           byte at offset 9, stores it back and masks the stored byte to 0xFF
 *           for the test. While that byte is nonzero the function returns
 *           untouched. When the countdown reaches zero it re-reads the same cell
 *           and increments work-object byte 1, which moves the dispatcher on to
 *           the next handler entry; unlike the neighbouring handler 0x801EEDCC
 *           it does not re-arm the countdown. Those are its only two stores: it
 *           reads no other cell, has no loop, and takes no arguments and returns
 *           nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startBurstInIdleEffectSlotsThenStepEffectSlotsAndAdvanceWorkByte1WhenCountdownExpiresBmagicMagic15003_801EED60(
    void) {
  func_801EF6F8();
  func_801EF8EC();
  if (--g_battle_work[9] == 0) {
    g_battle_work[1]++;
  }
}
