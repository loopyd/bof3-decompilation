#include "bof3/bof3.h"

void clearOwnerRecordFlag40(u8 id); /* @source 0x800AC844 */
u8 *findOwnerRecord(u8 id);         /* @source 0x800AC754 */
void func_8014DD3C(s32 entry);      /* @source 0x8014DD3C */
void func_8014D6B8(u32 resource);   /* @source 0x8014D6B8 */

/* Scratchpad work-record cursor: the record returned by findOwnerRecord is
 * published here so the later steps read it without a second lookup, and every
 * byte store after the request re-reads the cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x800C1ABC
 * @behavior Battle setup hook of this overlay, published into the shared hook
 * slot 0x801463A8 by the installer at 0x800C1A08. It strips the 0x40 flag of
 * owner record 0 through clearOwnerRecordFlag40, looks that same record up
 * through findOwnerRecord and publishes it into the scratchpad work-record
 * cursor 0x1F800044, requests entry 0xAA through the shared func_8014DD3C
 * helper, clears the published record bytes at offsets 0x2A and 0x48, and
 * requests resource 6 through the shared func_8014D6B8 helper. No jal in this
 * payload reaches it: the installer takes its address, so the main program calls
 * it through that slot. Takes no arguments and returns nothing; the 0x18-byte
 * frame keeps $ra across the four calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: both record helper sites at 0x800C1AC4 and 0x800C1ACC fill
 * their delay slot with `addu $a0,$zero,$zero`, so both are called with owner id
 * 0, the same opening pair as the exact siblings 0x800C1A6C of boss031/13 and
 * 0x800C1C64 of boss021/16. The `sw $v0,%lo(...)($at)` at 0x800C1AD8 stores the
 * findOwnerRecord result straight into 0x1F800044, and no 0x80146884 battle
 * record cell is written here. `addiu $a0,$zero,0xAA` at 0x800C1AE0 and
 * `addiu $a0,$zero,0x6` at 0x800C1AFC are this overlay's own request constants,
 * and the 0x8014DD3C request sits between the publication and the byte clears
 * while the 0x8014D6B8 request follows them, in the order the `jal` sites
 * appear. Each byte clear reloads the cursor first (`lui`/`lw` then `sb
 * $zero`) and the second clear fills the 0x8014D6B8 delay slot, so the source
 * evaluates the cell at both stores instead of keeping the pointer in a local;
 * the first store has nothing to fill its load slot with, hence the `nop` at
 * 0x800C1AEC. Prologue and return path place the boundary.
 */
void setupBattleWorkRecordBossBoss02213_800C1ABC(void) {
  clearOwnerRecordFlag40(0);
  D_1F800044 = findOwnerRecord(0);
  func_8014DD3C(0xAA);
  D_1F800044[0x2A] = 0;
  D_1F800044[0x48] = 0;
  func_8014D6B8(6);
}
