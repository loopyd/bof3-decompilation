#include "bof3/bof3.h"

/* Shared main-executable update pass. This body jal word encodes 0x800AF764 and
 * the two exact siblings of this family that reach the same address,
 * queueSlotSetByte1PublishRecordPointerCopyWorkWordTripletEveryFourthTickWhileByteABelow16ThenAdvanceWorkByte2At16BmagicMagic04403_801EF320
 * and
 * consumeRandomDecayWorkHalfwords2EAnd30AdvanceByte9By4AdvanceWorkByte2At10BmagicMagic04403_801F05AC,
 * call it the same way, so the prototype keeps its raw name. */
void func_800AF764(void);

/* PsyQ libc random source at 0x8017E3D4, the address the three jal words of
 * this body encode. Each answer is consumed as (rand() & 3) + 1 and stored, so
 * the declaration carries the library's return type. */
extern int rand(void);

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map.
 * The original materialises this cell with its own lui/lw pair before each of
 * the seven dereferences the body makes, including the ones that follow a plain
 * byte store through the base it just loaded, so the cell is a plain pointer
 * variable rather than a value pinned in a register: every store through the
 * loaded base may alias the cell itself and forces the reload the original
 * shows. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* @source 0x801F04F0
 * @behavior Entry 0 of the three-entry work-byte-2 handler table based at
 *           0x801F0A2C, which this overlay's byte 2 dispatcher
 *           dispatchWorkByte2HandlerThenDrawWhenActiveBmagicMagic04403_801F0478
 *           indexes with the scratchpad work object byte 0x2; no jal in this
 *           payload reaches this address, only that table word. It calls the
 *           shared main-executable update pass func_800AF764 first, then reads
 *           the work pointer cell 0x1F800044 and adds eight to the work object
 *           unsigned halfword 0x30 (lhu / addiu 8 / sh). Three more reads of the
 *           cell each store (rand() & 3) + 1, so work bytes 0x5D, 0x5E and 0x5F
 *           take independent random values in 1..4. A fifth read stores zero
 *           into work byte 9, a sixth stores 0x10 into work byte 0x0A, and a
 *           seventh increments work byte 2 modulo 256, the handler index the
 *           dispatcher reads, so this state block advances the run
 *           unconditionally while the sibling entries advance only on their own
 *           byte 9 or byte 0x0A condition. Takes no arguments, returns nothing
 *           and the 0x18-byte frame keeps only $ra at 0x10. Halfword 0x30 is the
 *           decay counter the sibling entries subtract one and two from, but the
 *           roles of work bytes 0x5D, 0x5E and 0x5F and of the byte 9 and byte
 *           0x0A pair are unestablished here, so the name stays offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperAdd8ToWorkHalfword30SeedRandomWorkBytes5d5e5fResetByte9SetByteATo10AdvanceWorkByte2BmagicMagic04403_801F04F0(void) {
  int r;
  u16 *halves;

  func_800AF764();
  halves = (u16 *)g_battle_work;
  halves[0x18] += 8;
  r = rand() & 3;
  g_battle_work[0x5D] = r + 1;
  r = rand() & 3;
  g_battle_work[0x5E] = r + 1;
  r = rand() & 3;
  g_battle_work[0x5F] = r + 1;
  g_battle_work[9] = 0;
  g_battle_work[0x0A] = 0x10;
  g_battle_work[2]++;
}
