#include "bof3/bof3.h"

/* Shared primary-state byte at 0x80146874 together with the sub-state byte at
 * 0x80146875. This body reads the pair as one halfword, which is the `lhu` the
 * original issues, and compares it with 0x0C06: primary state 6 with sub-state
 * 12. The main executable's own lifts store the two bytes separately, so the
 * halfword view is this overlay's alone and the cell keeps its address-derived
 * name.
 * @source 0x80146874 @kind unknown
 */
extern u16 D_80146874;

/* Count byte of the 0x140-byte work-record table based at 0x80145E90 in the
 * shared main-executable data segment. The exact plchar lifts that walk that
 * table read the same count byte, and this target's exact lift
 * setWorkByte29To5OnActiveWorkRecordsAndBaseWorld02Area08513_801F2FCC names
 * its base.
 * @source 0x80146254 @kind unknown
 */
extern u8 D_80146254;

/* The work records themselves. Byte 0x79 (0x80145F09) is the record state the
 * walk tests, and the words at row offsets 0x34, 0x38 and 0x3C (0x80145EC4,
 * 0x80145EC8, 0x80145ECC) are the triplet it copies. The original rebuilds
 * `lui at,%hi(<base>)` and adds the scaled row offset for each of the four
 * accesses, so each field keeps its own address-derived byte base rather than
 * one materialised row pointer.
 */
extern u8 D_80145F09[]; /* @source 0x80145F09 @kind data */
extern u8 D_80145EC4[]; /* @source 0x80145EC4 @kind data */
extern u8 D_80145EC8[]; /* @source 0x80145EC8 @kind data */
extern u8 D_80145ECC[]; /* @source 0x80145ECC @kind data */

/* Record published through the pointer cell at 0x80148210. The original loads
 * the cell once and lowers its halfword 0x0A by 2, so the cell is declared at
 * halfword width, the spelling the exact world03 lift
 * claimFrontEndRecordSetHandler85AndStoreWorkRecordIndexElseLowerHalfword0aBy2
 * uses for the same cell.
 * @source 0x80148210 @kind unknown
 */
extern u16* D_80148210;

/* Scratchpad work-object pointer cell. It is the named cell symbol this
 * target's exact lifts setFlag40UntilWorkByte9ExpiresThenClearSelectedRecords
 * (0x801F35A0), applyMainLookupRecord184ClearWorkByte24Bit0... (0x801F3AF8)
 * and invokeHelperWithWork (0x801F3BA4) declare: the original loads the pair
 * `lui $vN,0x1F80` / `lw $vN,0x44($vN)`, which is what a named cell keeps.
 */
extern u8* D_1F800044;

/* Shared main-executable services, both outside this payload: 0x801C1254 is
 * the transition helper and 0x8014D6B8 requests a resource by id. */
void func_801C1254(u32 arg0, u32 arg1); /* @source 0x801C1254 */
void func_8014D6B8(u32 resource); /* @source 0x8014D6B8 */

/* @source 0x801F37C0
 * @behavior Overlay step. It reads the shared primary-state and sub-state byte
 *           pair at 0x80146874 as one halfword; when that pair is 0x0C06
 *           (state 6, sub-state 12) it clears bit 0x40 of byte 0 of the work
 *           record published at the scratchpad pointer cell 0x1F800044, runs
 *           the shared transition helper 0x801C1254 with 2 and 2, requests
 *           resource 1 through 0x8014D6B8 and returns without touching the
 *           countdown. Otherwise it takes the row count from D_80146254 and,
 *           while that count is not zero, walks the 0x140-byte work records
 *           from index 0: each record whose state byte at row offset 0x79 reads
 *           2 has its word triplet at row offsets 0x34, 0x38 and 0x3C copied
 *           into words 0x34, 0x38 and 0x3C of the single work record the
 *           scratchpad cell published; the walk then lowers by 2 the halfword
 *           0x0A of the record published at 0x80148210.
 *
 *           Shape facts from the original bytes: the arm-1 bit clear goes
 *           through the cell symbol, so its mask store fills the first call's
 *           delay slot the way the exact sibling 0x801F3AF8 does; the row count
 *           is a byte, the walk index is a 32-bit counter masked to a byte for
 *           both the scaled row offset and the loop bound, the byte index idiom
 *           the exact sibling 0x801F2FCC uses; the walk is a guarded do-while so
 *           the count test is the only entry test; the cell is loaded once
 *           inside that guard; and the compared state is held in a local so its
 *           constant is materialised before the cell load. No jal in the
 *           payload names this function: only the word at 0x801F42A0 holds its
 *           address, inside the payload's trailing script blob, so it is
 *           reached from that blob rather than from code. Its 0x18-byte frame
 *           saves $ra at 0x10 and its single plain return path places the
 *           boundary.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearFlag40AndRequestResourceOnMode6Substate12ElseCopyStatus2WorkRecordTripletAndTickHalfword0aWorld02Area08513_801F37C0(
    void) {
  u8* work;
  u8 count;
  u32 i;
  u32 offset;
  u32 state;

  if (D_80146874 == 0xC06) {
    D_1F800044[0] &= ~0x40;
    func_801C1254(2, 2);
    func_8014D6B8(1);
  } else {
    i = 0;
    count = D_80146254;
    if (count != 0) {
      state = 2;
      work = D_1F800044;
      do {
        offset = (u8)i * 0x140;
        if (D_80145F09[offset] == state) {
          *(u32*)&work[0x34] = *(u32*)(D_80145EC4 + offset);
          *(u32*)&work[0x38] = *(u32*)(D_80145EC8 + offset);
          *(u32*)&work[0x3C] = *(u32*)(D_80145ECC + offset);
        }
        i++;
      } while ((u8)i < count);
    }
    D_80148210[5] -= 2;
  }
}
