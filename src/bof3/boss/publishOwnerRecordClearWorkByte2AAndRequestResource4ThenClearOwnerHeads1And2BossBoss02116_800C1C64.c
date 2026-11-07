#include "bof3/bof3.h"

void func_800AC844(s32 arg0);
u8  *func_800AC754(s32 arg0);
void func_8014D6B8(s32 arg0);
void func_800AC934(s32 arg0);

/* Scratchpad work-record cursor: the record loaded through func_800AC754 is
 * published here so later steps read it without a second lookup. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind bss */

/* @source 0x800C1C64
 * @behavior Owner-zero battle entry of this overlay. Takes the record of owner
 * id 0 from the shared work table through func_800AC754 after stripping its
 * 0x40 flag through func_800AC844, publishes that record into the scratchpad
 * work-record cursor 0x1F800044, clears the record byte at offset 0x2A, then
 * requests resource 4 through func_8014D6B8 and clears the owner-record heads
 * for ids 1 and 2 through the helper at 0x800AC934 in that order. Takes no
 * arguments and returns nothing; the 0x18-byte frame keeps $ra across the
 * calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: the two `jal` sites at 0x800C1C6C and 0x800C1C74 both fill
 * their delay slot with `addu $a0,$zero,$zero`, so both record helpers are
 * called with a zero owner id, the same opening pair as the exact sibling
 * 0x800C1A6C of boss031/13. `addiu $a0,$zero,0x4` at 0x800C1C7C is this
 * overlay's own request constant where that sibling passes 1. The sw at
 * 0x800C1C84 stores $v0 straight into 0x1F800044, and the `sb $zero,0x2A($v0)`
 * in the func_8014D6B8 delay slot writes through the same live $v0, so one
 * local carries the record and the byte is cleared before that request; the
 * sibling reloads the cursor instead because its store runs after its request.
 * This overlay publishes no 0x80146884 battle-record cell. The two
 * func_800AC934 sites at 0x800C1C90 and 0x800C1C98 carry `addiu $a0,$zero,0x1`
 * and `addiu $a0,$zero,0x2` in their delay slots, and 0x800AC934 is the reviewed
 * exact lift clearOwnerRecordHead, which clears the five leading bytes of the
 * record selected by that id, so those calls clear the owner-record heads of
 * ids 1 and 2 rather than request anything. Prologue and return path place the
 * boundary.
 */
void publishOwnerRecordClearWorkByte2AAndRequestResource4ThenClearOwnerHeads1And2BossBoss02116_800C1C64(
    void) {
  u8 *record;

  func_800AC844(0);
  record = func_800AC754(0);
  D_1F800044 = record;
  record[0x2A] = 0;
  func_8014D6B8(4);
  func_800AC934(1);
  func_800AC934(2);
}
