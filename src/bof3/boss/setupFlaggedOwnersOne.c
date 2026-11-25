#include "bof3/bof3.h"

/* Owner-record helpers of the main executable, shared by the battle and boss
 * overlays of this archive family: 0x800AC844 clears bit 0x40 in the leading
 * byte of the record selected by the owner id, and 0x800AC870 starts the record
 * band its second argument points at for that owner. The exact sibling
 * setupBattleWorkRecordForOwnerZeroThenCopyHalfwordPair58And5ABossBoss00816_800C2028
 * declares the same pair, and this target's generated undefined_funcs_auto rows
 * bind both address-anchored names. */
void func_800AC844(u8 id);                     /* @source 0x800AC844 */
void func_800AC870(u8 id, u8* band, s32 mode); /* @source 0x800AC870 */

/* Shared battle flag byte above the battle flag halfword 0x801462E8; its bit
 * 0x1 and bit 0x2 select the two owner records this setup hook starts. */
extern u8 D_801462ED; /* @source 0x801462ED @kind unknown */

/* The two enemy work record bands of the shared battle segment. 0x801EB748 is
 * exactly one 0x118-byte record stride above 0x801EB630, so these are owner
 * records 1 and 2 of the same run; only their addresses are taken here. */
extern u8 D_801EB630[]; /* @source 0x801EB630 @kind unknown */
extern u8 D_801EB748[]; /* @source 0x801EB748 @kind unknown */

/* @source 0x800C2F74
 * @behavior Battle setup hook of this overlay, the second member of the handler
 *           trio the exact installer publishBattleHooks28
 *           publishes into the shared battle hook slot 0x801463A8; no `jal` in
 *           this payload reaches it and the only word naming it is that
 *           installer's store, so the main program calls it through the slot.
 *           It takes one local pointer to the shared battle flag byte
 *           0x801462ED and tests that byte's low bits. When bit 0x1 is set it
 *           starts the enemy work record band based at 0x801EB630 for owner 1
 *           through func_800AC870, and when bit 0x2 is set it starts the next
 *           band at 0x801EB748, one 0x118-byte record stride higher, for owner
 *           2. Each band is then closed unconditionally by func_800AC844 with
 *           that band's owner id 1 or 2, the helper the exact family sibling
 *           setupBattleWorkRecordForOwnerZeroThenCopyHalfwordPair58And5ABossBoss00816_800C2028
 *           documents as clearing bit 0x40 in the leading byte of the record
 *           selected by the owner id, so a band is only started when its flag
 *           bit asks for it while the owner record is settled on every path.
 *           Both helpers may write the flag byte, so it is reloaded before the
 *           second test; the one saved register $s0 holds the byte pointer and
 *           the 0x18-byte frame keeps $ra at 0x14. Takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setupFlaggedOwnersOne(
    void) {
  u8* flags;

  flags = &D_801462ED;
  if ((*flags & 0x1) != 0) {
    func_800AC870(1, D_801EB630, 0);
  }
  func_800AC844(1);
  if ((*flags & 0x2) != 0) {
    func_800AC870(2, D_801EB748, 0);
  }
  func_800AC844(2);
}
