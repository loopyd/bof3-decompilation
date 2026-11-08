#include "bof3/bof3.h"

/* Owner-record helpers of the main executable, shared by the battle and boss
 * overlays of this archive family: 0x800AC844 clears bit 0x40 in the leading
 * byte of the record selected by the owner id, and 0x800AC754 returns the base
 * of the first 0x98-byte work record whose kind byte is 7 and whose owner byte
 * matches that id. This target's generated undefined_funcs_auto rows bind the
 * address-anchored names. */
void func_800AC844(u8 id); /* @source 0x800AC844 */
u8 *func_800AC754(u8 id);  /* @source 0x800AC754 */

/* Shared main-executable lookup helper: its exact lift at 0x8014DD3C applies
 * the main lookup record whose id matches its argument to the active work
 * record, so the argument here is a record id. This target's generated
 * undefined_funcs_auto row binds the address-anchored name. */
void func_8014DD3C(s32 entry); /* @source 0x8014DD3C */

/* Resource request helper of the main executable, reached through the same
 * generated undefined_funcs_auto row; the sibling lifts of this target declare
 * the same prototype and pass a resource id. */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* Halfword pair of the battle-side record block that this overlay copies into
 * the published work record at +0x58 and +0x5A. The two cells are adjacent, so
 * the source reads them as one record's leading halfwords; the role of that
 * record block is not established, so both stay address-anchored. */
extern u16 D_801EB688; /* @source 0x801EB688 @kind unknown */
extern u16 D_801EB68A; /* @source 0x801EB68A @kind unknown */

/* Scratchpad work-record cursor: the record returned by func_800AC754 is
 * published here so the later steps read it without a second lookup. The
 * original reloads the cell after the lookup call and again after the resource
 * request, so the mark store addresses the named cell and the halfword run then
 * keeps one loaded base. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x800C2210
 * @behavior Battle setup hook of this overlay for owner record 2, published
 * into the shared battle hook slot 0x801463A8 by the exact installer
 * publishBattleHooksBossBoss00816_800C2184; no `jal` in this payload reaches
 * it, so the main program calls it through that slot. It clears the 0x40 flag
 * of owner record 2 through func_800AC844, looks the same record up through
 * func_800AC754 and publishes it into the scratchpad work-record cursor
 * 0x1F800044, applies main lookup record 0x83 through func_8014DD3C, marks the
 * published record by storing 1 into its byte 0x2A, requests resource 0 through
 * func_8014D6B8, and then copies the adjacent halfword pair
 * D_801EB688/D_801EB68A into that record at 0x58 and 0x5A. Takes no arguments
 * and returns nothing; the 0x18-byte frame keeps $ra at 0x10 across the four
 * calls.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Byte evidence: this 0x70-byte body is the owner-2 member of the setup group
 * whose owner-0 (0x800C2028) and owner-3 (0x800C230C) members had already been
 * lifted exact, and it differs from them in the two owner immediates plus the
 * four instructions of the byte 0x2A mark. The mark reloads the scratchpad cell
 * into $v1, materialises 1 into $v0 and issues its `sb` in the delay slot of the
 * resource request, which is the form the exact sibling
 * publishOwnerRecordAndRequestResourceThenSetWorkByte2ABossBoss03113_800C1A6C
 * reached for the same byte, written there after its call. The unlifted owner-1
 * body at 0x800C2114 is this same body with 1 in those two argument words, so it
 * is shape evidence and not a twin; a payload scan of every extracted binary,
 * with the four call words masked, found no byte-identical span anywhere.
 */
void setupBattleWorkRecordForOwnerTwoThenSetWorkByte2AAndCopyHalfwordPair58And5ABossBoss00816_800C2210(
    void) {
  u8 *record;

  func_800AC844(2);
  D_1F800044 = func_800AC754(2);
  func_8014DD3C(0x83);
  D_1F800044[0x2A] = 1;
  func_8014D6B8(0);
  record = D_1F800044;
  FIELD_REF(u16, record, 0x58u) = D_801EB688;
  FIELD_REF(u16, record, 0x5Au) = D_801EB68A;
}
