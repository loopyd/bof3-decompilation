#include "bof3/bof3.h"

/* Base of the 0x140-stride local work records the battle code keeps for the
 * three party battlers. Byte +0x00 holds the record activity bit 0x1, byte
 * +0x05 the battler index clearBattlerActionFlags takes, byte +0x08 the region
 * selector this hook adds to, and halfword +0x80 the party flags whose bit
 * 0x4000 picks the alternate region base of the record. The original addresses
 * every one of those bytes through the array symbol rather than through the
 * record pointer, so the fields keep the array spelling here. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */

/* Scratchpad work-record pointer cell: the battle code publishes the record it
 * is working on here. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battler service of the main executable, lifted exactly at 0x801DD14C
 * as clearBattlerActionFlags: it clears the action scratch flags of the battler
 * whose index it is given and drops that battler from the ranked owner list. */
void clearBattlerActionFlags(u8 arg0); /* @source 0x801DD14C */

/* Shared main-executable region helper, the same family as 0x8014D664 which
 * another overlay documents as taking a work-record byte, the fixed 0x800F0800
 * source base and a size. Its argument roles beyond the first selector byte are
 * unknown, so the address-anchored name and the u8/u32/s32 prototype stay. */
void func_8014D5F0(u8 arg0, u32 arg1, s32 arg2); /* @source 0x8014D5F0 */

/* Shared mode-tuple service of the main executable: 0x801DDF28 seeds the
 * three-byte battle mode tuple with 5, 3 and 0, the pairing the other boss
 * overlays of this archive family record as initModeTuple530. */
void initModeTuple530(void); /* @source 0x801DDF28 */

/* Shared scenario progress byte the boss overlays of this family stamp before
 * they seed a mode tuple. */
extern u8 g_ScenarioProgress; /* @source 0x80146864 @kind unknown */

/* @source 0x800C2984
 * @behavior Battle-step hook of this overlay at 0x800C2984, the routine its
 * installer 0x800C2710 stores into the shared hook slot 0x801463A4; no jal in
 * this payload names it and no data word holds its address, so the battle step
 * reaches it through that slot. It refreshes the three 0x140-stride local work
 * records that start at 0x80145E90, one block per party battler, and each block
 * is the same four steps in the same order: take the record address into a
 * pointer register and test the record activity bit 0x1 through it; when that
 * bit is set, clear the battler action flags through 0x801DD14C with the record
 * byte +0x05 index, read the party-flag halfword at +0x80, publish the record
 * address into the scratchpad work cell 0x1F800044, and call 0x8014D5F0 with
 * 0x800F0800 and 0x1800, selecting the record byte +0x08 with 0x1C added when
 * bit 0x4000 of that halfword is set and with 0x04 added when it is clear. The
 * halfword load precedes the scratchpad store and the store precedes the 0x4000
 * test in source order, so the published record is visible before the flag
 * decides the region. The three blocks carry no shared gate: unlike the
 * boss055/16 sibling 0x800C24E0, which tests bit 0x2 of 0x80146328 around the
 * same three blocks and runs initModeTuple520 when that bit is clear, this hook
 * tests each record on its own and has a single tail. After the third block it
 * stores 4 into the shared scenario progress byte 0x80146864 and runs
 * initModeTuple530 at 0x801DDF28. The stored 4 is materialised once in the
 * delay slot of the third record test and again after the region call, because
 * that call clobbers the register. The body takes no arguments, keeps the
 * family 0x18-byte frame with $ra at 0x14 and $s0 at 0x10, and returns nothing.
 * Its three record blocks are the same instructions as this payload's
 * 0x800C2D7C, which was still on its asm row when this comparison was
 * measured: comparing the two spans of the
 * original payload leaves five differing words, three absolute jump targets
 * that move by the 0x3F8 offset between them, one branch offset, and the one
 * delay slot that carries the stored literal. That twin's tail stamps 0x0F,
 * 0x07 and 0x14 into 0x80146875 under the three bits of 0x801462ED instead of
 * setting the scenario progress, so it is shape and naming evidence for this
 * body, not a clone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshActivePartyRecords(
    void) {
  u8 *rec;
  u16 flag;

  rec = &D_80145E90[0x0];
  if ((rec[0] & 0x1) != 0) {
    clearBattlerActionFlags(D_80145E90[0x5]);
    flag = FIELD_REF(u16, D_80145E90, 0x80);
    g_battle_work = rec;
    if ((flag & 0x4000) != 0) {
      func_8014D5F0(D_80145E90[0x8] + 0x1C, 0x800F0800u, 0x1800);
    } else {
      func_8014D5F0(D_80145E90[0x8] + 0x4, 0x800F0800u, 0x1800);
    }
  }
  rec = &D_80145E90[0x140];
  if ((rec[0] & 0x1) != 0) {
    clearBattlerActionFlags(D_80145E90[0x145]);
    flag = FIELD_REF(u16, D_80145E90, 0x1C0);
    g_battle_work = rec;
    if ((flag & 0x4000) != 0) {
      func_8014D5F0(D_80145E90[0x148] + 0x1C, 0x800F0800u, 0x1800);
    } else {
      func_8014D5F0(D_80145E90[0x148] + 0x4, 0x800F0800u, 0x1800);
    }
  }
  rec = &D_80145E90[0x280];
  if ((rec[0] & 0x1) != 0) {
    clearBattlerActionFlags(D_80145E90[0x285]);
    flag = FIELD_REF(u16, D_80145E90, 0x300);
    g_battle_work = rec;
    if ((flag & 0x4000) != 0) {
      func_8014D5F0(D_80145E90[0x288] + 0x1C, 0x800F0800u, 0x1800);
    } else {
      func_8014D5F0(D_80145E90[0x288] + 0x4, 0x800F0800u, 0x1800);
    }
  }
  g_ScenarioProgress = 0x4;
  initModeTuple530();
}
