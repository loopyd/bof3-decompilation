#include "bof3/bof3.h"

/* Shared battle flag byte: bit 0x2 gates this overlay's work-record refresh and
 * is tested the same way by the sibling overlays of this archive family. */
extern u8 D_80146328; /* @source 0x80146328 @kind unknown */

/* Shared battle step byte. The other step handlers of this archive family store
 * 4, 7 and 0x14 into it immediately before the mode-tuple call; this handler
 * stores 0x0F, the value that accompanies mode tuple 530. */
extern u8 D_80146875; /* @source 0x80146875 @kind unknown */

/* Shared battle byte the handlers of this family stamp while they run; the
 * sibling boss013/16 handler writes it after its own flag-bit-two work and the
 * boss022/13 entry setup writes 0x6A into it. */
extern u8 D_80145029; /* @source 0x80145029 @kind unknown */

/* Base of the 0x140-stride local work records the battle code keeps for the
 * three party battlers. Byte +0x00 holds the record's activity bit 0x1, byte
 * +0x05 the battler index clearBattlerActionFlags takes, byte +0x08 the region
 * selector this handler adds to, and halfword +0x80 the party flags whose bit
 * 0x4000 picks the record's other region base. The original addresses every one
 * of those bytes through the array symbol rather than through the record
 * pointer, so the fields keep the array spelling here. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind unknown */

/* Scratchpad work-record pointer cell: the battle code publishes the record it
 * is working on here and every access reaches it through this cell. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battler service of the main executable, lifted exactly at 0x801DD14C
 * as clearBattlerActionFlags: it clears the action scratch flags of the battler
 * whose index it is given and drops that battler from the ranked owner list. */
void clearBattlerActionFlags(u8 arg0); /* @source 0x801DD14C */

/* Shared main-executable region helper, the same family as 0x8014D664 which
 * another overlay documents as taking a work-record byte, the fixed 0x800F0800
 * source base and a size. Its argument roles beyond the first selector byte are
 * unknown, so the address-anchored name and the header's u8/u32/s32 prototype
 * stay. */
void func_8014D5F0(u8 arg0, u32 arg1, s32 arg2); /* @source 0x8014D5F0 */

/* Shared mode-tuple services of the main executable. 0x801DDF00 seeds the
 * three-byte battle mode tuple with 5, 2 and 0 (initModeTuple520) and
 * 0x801DDF28 seeds it with 5, 3 and 0 (initModeTuple530); the in-repo lifts of
 * the other boss overlays record both. */
void initModeTuple520(void); /* @source 0x801DDF00 */
void initModeTuple530(void); /* @source 0x801DDF28 */

/* @source 0x800C24E0
 * @behavior Battle-hook step handler of this overlay, the body
 * publishBattleHooksBossBoss05516_800C24A0 publishes into the shared hook slot
 * 0x801463A4, and so the routine the battle step calls through that slot. While
 * bit 0x2 of the shared battle flag byte 0x80146328 is set it refreshes the
 * three 0x140-stride local work records that start at 0x80145E90, one block per
 * party battler, and each block is the same four steps in the same order: take
 * the record's own address into a pointer register and test the record's
 * activity bit 0x1 through it; when that bit is set, clear the battler's action
 * flags through clearBattlerActionFlags with the record's byte +0x05 index,
 * read the record's party-flag halfword at +0x80, publish the record address
 * into the scratchpad work cell 0x1F800044, and call func_8014D5F0 with
 * 0x800F0800 and 0x1800, selecting the record's byte +0x08 with 0x1C added when
 * bit 0x4000 of that halfword is set and with 0x04 added when it is clear. The
 * halfword load precedes the scratchpad store and the store precedes the 0x4000
 * test in source order, so the published record is visible before the flag
 * decides the region. After the third block it stores 0x0F into the shared step
 * byte 0x80146875, runs initModeTuple530 and stamps 0xFF into 0x80145029. With
 * bit 0x2 clear it runs initModeTuple520 instead and refreshes nothing. The
 * body takes no arguments, keeps the family's 0x18-byte frame with $ra at 0x14
 * and $s0 at 0x10, and returns nothing; the frame's $s0 spill fills the delay
 * slot of the flag-bit-two branch because both paths restore it. Neither this
 * address nor the hook slot word appears as a word of this payload, so the main
 * program enters the handler through the published hook only.
 * @status exact
 * @match 100.00
 * @residual none
 */
void refreshActivePartyRecordsThenSetStepByte0FAndInitModeTuple530BossBoss05516_800C24E0(
    void) {
  u8 *rec;
  u16 flag;

  if ((D_80146328 & 0x2) != 0) {
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
    D_80146875 = 0xF;
    initModeTuple530();
    D_80145029 = 0xFF;
  } else {
    initModeTuple520();
  }
}
