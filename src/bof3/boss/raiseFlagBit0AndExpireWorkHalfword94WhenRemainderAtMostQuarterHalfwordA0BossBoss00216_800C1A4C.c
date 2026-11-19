#include "bof3/bof3.h"

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Shared battle flag byte the boss overlays raise one bit of each. */
extern u8 D_801462ED; /* @source 0x801462ED @kind unknown */

/* @source 0x800C1A4C
 * @behavior Entry 3 of the twelve-word handler run based at 0x800C1FEC, the
 * word 0x800C1FF8 of the payload data row D_800C1F9C: the dispatcher
 * dispatchStateByte2Table1FECBossBoss00216_800C18A0 indexes that run from
 * work-record byte 2 and calls the selected entry with no arguments, and no jal
 * in the payload names this body, so the prologue and the return path alone
 * place its boundary. It loads the current enemy work record pointer published
 * at 0x801EB4E8, forms the record halfword at 0x94 minus its signed halfword at
 * 0xF8, and compares that signed remainder against the record halfword at 0xA0
 * shifted right by two. While a quarter of that halfword is still below the
 * remainder it returns without touching any state; otherwise it sets bit 0 of
 * the shared battle flag byte 0x801462ED and stores 0xFFFF into the record
 * halfword at 0x94. The neighboring entry 0x800C1A00 of the same run compares
 * that halfword against 0xFFFF before it publishes byte 3 to the shared state
 * byte 0x80146375 and clears work-object byte 4, so 0xFFFF is the value the
 * payload reads as work expired. The roles of the 0x94, 0xF8 and 0xA0 fields
 * are not established here, so they stay named by offset. The 0x54-byte body
 * was byte-identical to the already-exact sibling at 0x800C1D14 of
 * BIN/BOSS/BOSS033.EMI#16 in the payload comparison that seeded it, so this
 * body is that sibling's statements with this target's own bindings. It takes
 * no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlagBit0AndExpireWorkHalfword94WhenRemainderAtMostQuarterHalfwordA0BossBoss00216_800C1A4C(
    void) {
  u8 *work;
  u8 *flags;
  s32 remainder;

  work = D_801EB4E8;
  remainder = FIELD_REF(u16, work, 0x94u) - FIELD_REF(s16, work, 0xF8u);
  if ((FIELD_REF(u16, work, 0xA0u) >> 2) < remainder) {
    return;
  }
  flags = &D_801462ED;
  *flags |= 1;
  FIELD_REF(u16, work, 0x94u) = 0xFFFFu;
}
