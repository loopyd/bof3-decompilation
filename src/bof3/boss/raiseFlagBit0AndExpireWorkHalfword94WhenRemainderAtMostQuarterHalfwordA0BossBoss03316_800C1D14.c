#include "bof3/bof3.h"

/* Current enemy work record pointer the main program publishes at 0x801EB4E8
 * and its battle overlays read back. */
extern u8 *D_801EB4E8; /* @source 0x801EB4E8 @kind unknown */

/* Shared battle flag byte the boss overlays raise one bit of each. */
extern u8 D_801462ED; /* @source 0x801462ED @kind unknown */

/* @source 0x800C1D14
 * @behavior Entry 1 of the three-word handler table based at 0x800C27A0, whose
 * entry 0 is the already-exact sibling at 0x800C1CDC and whose entry 2 is the
 * inert noopHandlerBossBoss03316_800C1D68: the work-step dispatcher
 * func_800C1CA4 masks its argument with 0xFF, scales it by four and calls that
 * run, and the installer func_800C1848 publishes that dispatcher into byte
 * offset 0xE4 of the work record. It takes no arguments and returns nothing. It
 * loads the current enemy work record pointer published at 0x801EB4E8, forms
 * the record halfword at 0x94 minus its signed halfword at 0xF8, and compares
 * that signed remainder against the record halfword at 0xA0 shifted right by
 * two. While a quarter of that halfword is still below the remainder it returns
 * without touching any state; otherwise it sets bit 0 of the shared battle flag
 * byte 0x801462ED and stores 0xFFFF into the record halfword at 0x94, the value
 * the sibling at 0x800C1CDC reads as work expired. The roles of the 0x94, 0xF8
 * and 0xA0 fields are not established here, so they stay named by offset.
 * No jal in the payload names 0x800C1D14: its only in-image reference is word
 * 1 of the 0x800C27A0 run.
 * @status exact
 * @match 100.00
 * @residual none
 */
void raiseFlagBit0AndExpireWorkHalfword94WhenRemainderAtMostQuarterHalfwordA0BossBoss03316_800C1D14(
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
