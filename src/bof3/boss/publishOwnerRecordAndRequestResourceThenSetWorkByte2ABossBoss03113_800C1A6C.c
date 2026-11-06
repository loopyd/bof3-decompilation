#include "bof3/bof3.h"

void clearOwnerRecordFlag40(u8 id); /* @source 0x800AC844 */
u8 *findOwnerRecord(u8 id);         /* @source 0x800AC754 */
void func_8014D6B8(u32 resource);   /* @source 0x8014D6B8 */

/* Shared battle-record pointer the main program hands to the overlays; the two
 * byte offsets other lifts read through it (0x93, 0x94) prove it points at the
 * 0x98-byte work record. */
extern u8 *D_80146884; /* @source 0x80146884 @kind unknown */
/* Scratchpad work-record pointer cell. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind bss */

/* @source 0x800C1A6C
 * @behavior Owner-zero battle entry of this overlay. Takes the record of owner
 * id 0 from the shared work table, strips its 0x40 flag through
 * clearOwnerRecordFlag40, and then publishes the same record pointer twice: into
 * the shared battle-record cell 0x80146884 and into the scratchpad work-record
 * cursor 0x1F800044. It requests resource 1 through func_8014D6B8 and, after
 * that call, marks the freshly published work record by storing 1 into its byte
 * at offset 0x2A. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: the two `jal` sites at 0x800C1A74 and 0x800C1A7C both fill
 * their delay slot with `addu $a0,$zero,$zero`, so both helpers are called with a
 * zero owner id; the same pair is what the setup routine of the sibling
 * installer publishBattleHooksBossBoss05216_800C1B0C describes (0x800AC844
 * then 0x800AC754, result published to 0x80146884 and to 0x1F800044, then one
 * resource request), and this function adds the reload-then-store tail. The
 * request constant is this overlay's own: `addiu $a0,$zero,0x1` at 0x800C1A98
 * where the nearest sibling passes 5. The pointer stays in $v0 across both
 * `sw $v0,%lo(...)($at)` stores, so one local carries it. The tail reloads the
 * scratchpad cell after the call (`lui $v1,0x1F80` / `lw $v1,0x44($v1)`), which
 * is the reload a non-volatile cell access after a call emits; the byte offset
 * is 0x2A and the stored constant is materialized into $v0 after that load.
 */
void publishOwnerRecordAndRequestResourceThenSetWorkByte2ABossBoss03113_800C1A6C(void) {
  u8 *record;

  clearOwnerRecordFlag40(0);
  record = findOwnerRecord(0);
  D_80146884 = record;
  D_1F800044 = record;
  func_8014D6B8(1);
  D_1F800044[0x2A] = 1;
}
