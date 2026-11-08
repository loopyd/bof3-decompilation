#include "bof3/bof3.h"

void clearOwnerRecordFlag40(u8 id); /* @source 0x800AC844 */
u8 *findOwnerRecord(u8 id);         /* @source 0x800AC754 */
void func_8014D6B8(u32 resource);   /* @source 0x8014D6B8 */

/* Shared battle-record pointer the main program hands to the overlays; the
 * overlay setup path publishes the owner-0 record here so the battle step reads
 * it without a second table lookup. */
extern u8 *D_80146884; /* @source 0x80146884 @kind unknown */
/* Scratchpad work-record cursor: the same pointer is published here so the
 * later step handlers reach the record without re-running the owner lookup. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* @source 0x800C1BEC
 * @behavior Battle setup hook of this overlay. Strips the 0x40 flag of owner
 * record 0 through clearOwnerRecordFlag40, looks that same record up through
 * findOwnerRecord and publishes the returned pointer twice: into the shared
 * battle-record cell 0x80146884 and into the scratchpad work-record cursor
 * 0x1F800044. It then requests resource 0 through the shared func_8014D6B8
 * helper and returns without touching the published record again. The installer
 * at 0x800C19C8 materializes this address as `lui`/`addiu` and stores it into
 * the shared hook slot 0x801463A8, so the main program calls it through that
 * slot; no jal in this payload reaches it. Takes no arguments and returns
 * nothing, and the 0x18-byte frame keeps $ra at 0x10 across the three calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: both record helper sites at 0x800C1BF4 and 0x800C1BFC fill
 * their delay slot with `addu $a0,$zero,$zero`, so both are called with owner id
 * 0, the same opening pair as the exact siblings 0x800C1ABC of boss022/13,
 * 0x800C1A6C of boss031/13 and 0x800C1C64 of boss021/16. The findOwnerRecord
 * result stays in $v0 across the two `sw $v0,%lo(...)($at)` stores at
 * 0x800C1C08 and 0x800C1C10, so one local carries it, and the scratchpad store
 * uses the plain `lui $at,0x1F80` / `sw $v0,0x44($at)` form a non-volatile cell
 * access emits. `addu $a0,$zero,$zero` in the 0x800C1C18 delay slot is this
 * overlay own request constant, zero, where the nearest sibling passes 1. No
 * statement follows that call: the return path reloads $ra and the nop at
 * 0x800C1C28 is the delay slot of the final `jr`.
 */
void publishOwnerRecordAndRequestResourceZeroBossBoss03216_800C1BEC(void) {
  u8 *record;

  clearOwnerRecordFlag40(0);
  record = findOwnerRecord(0);
  D_80146884 = record;
  D_1F800044 = record;
  func_8014D6B8(0);
}
