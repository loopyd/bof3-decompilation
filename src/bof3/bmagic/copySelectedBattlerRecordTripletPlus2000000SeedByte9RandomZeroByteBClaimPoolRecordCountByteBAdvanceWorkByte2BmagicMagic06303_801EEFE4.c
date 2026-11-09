#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's
 * map as g_battle_work, the name the exact bmagic lifts of this family use for
 * the same main-executable cell. The original materialises the cell with its own
 * lui / lw pair before each of the seven dereferences the body makes, because
 * every read after the first is separated from the previous one by a store
 * through the pointer just loaded. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battler work records selected by the index this body reads: the party
 * entries hold 0x140-byte rows at 0x80145E90 and the enemy entries 0x118-byte
 * rows at 0x801EB2E8, the split the exact sibling lift of emi/bmagic/magic094
 * applies to the same two bases for the same three-record threshold. Both
 * addresses sit outside this overlay image, so the reference binds through the
 * target's generated undefined-symbol table, which Splat rebuilds from these
 * declarations; neither base needs a row in this target's symbols.txt. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */
extern u8 D_801EB2E8[]; /* @source 0x801EB2E8 @kind unknown */

/* Record pool at 0x801F0454: 0x60 records of stride 0x20 whose byte 0 carries
 * the in-use bit reserveFirstFreeRecordBmagicMagic06303_801F0354 sets and whose
 * bytes 0, 1 and 2 the reset at 0x801EECF8 clears. This body fills byte 1 of the
 * record it claims and the pointer word at +0x1C of that same record, and the
 * original reaches both as `lui $at,%hi(ADDR)` / `addu $at,$at,index*0x20` /
 * `sb|sw %lo(ADDR)($at)` over the flat addresses 0x801F0455 and 0x801F0470, so
 * each addressed field stays an offset of this one named base. */
extern u8 magic06303RecordFlags[]; /* @source 0x801F0454 @kind data */

/* Claimed-record allocator of this overlay: returns the index of the first pool
 * record whose flags byte has bit 0 clear, or 0xFF once all 0x60 are occupied,
 * which is the value this body tests. It sits behind a jal in the original, so
 * it keeps an extern prototype. */
extern u8 reserveFirstFreeRecordBmagicMagic06303_801F0354(void); /* @source 0x801F0354 */

/* PsyQ C runtime at 0x8017E3D4. Only the low four bits of the answer are used,
 * as the one-based byte written into work byte 9. */
extern int rand(void);

/* @source 0x801EEFE4
 * @behavior Entry 0 of the five-word code pointer run at 0x801EEC04 whose other
 *           words are 0x801EF110, 0x801EF190, 0x801EF1C8 and 0x801EF2E4, the
 *           table the overlay dispatcher at 0x801EEF74 copies into its own frame
 *           and indexes by work byte 2; no jal word of this payload names this
 *           address, so it is reached through that run alone. Reads the
 *           scratchpad work-object pointer cell at 0x1F800044 once before it
 *           creates its 0x18-byte frame, and the work object byte 4 there
 *           selects the source record: below three the 0x140-byte-stride party
 *           records at 0x80145E90, otherwise the 0x118-byte-stride enemy records
 *           at 0x801EB2E8, with the scaled index kept in one register and the
 *           base in another so the single add that forms the record address sits
 *           after both arms. Copies the selected record words 0x34, 0x38 and
 *           0x3C into the same three words of the work object, adding 0x2000000
 *           to the third copy, then seeds work byte 9 with one plus the low four
 *           bits of rand(), clears work byte 0xB and increments work byte 2, the
 *           handler selector the dispatcher reads. It then claims one pool
 *           record through reserveFirstFreeRecordBmagicMagic06303_801F0354 and,
 *           unless the answer is 0xFF, stamps the claimed record byte 1 with 1
 *           and its word 0x1C with the work-object pointer and increments work
 *           byte 0xB once more, so that byte ends as the number of records this
 *           step claimed, 0 or 1. Every one of those accesses re-reads the
 *           pointer cell at its own site and the frame keeps only $ra at 0x10.
 *           Takes no arguments, returns nothing and reads or writes no other
 *           state; the roles of work bytes 4 and 0xB beyond the selection index
 *           and the claimed-record count their call sites show, and of work
 *           word 0x3C beyond the record word it copies, stay unestablished.
 * @status exact
 * @match 100.00
 * @residual none
 */
void copySelectedBattlerRecordTripletPlus2000000SeedByte9RandomZeroByteBClaimPoolRecordCountByteBAdvanceWorkByte2BmagicMagic06303_801EEFE4(void) {
  u8 *work;
  u8 *target;
  u8 *record;
  u8 *claims;
  u8 *count;
  u8 index;
  u8 claimed;
  u32 slot;
  int random;

  work = g_battle_work;
  index = work[4];
  if (index < 3) {
    record = (u8 *)D_80145E90 + index * 0x140u;
  } else {
    record = D_801EB2E8 + index * 0x118u;
  }
  target = g_battle_work;
  *(u32 *)(target + 0x34) = *(u32 *)(record + 0x34);
  *(u32 *)(target + 0x38) = *(u32 *)(record + 0x38);
  *(u32 *)(target + 0x3C) = *(u32 *)(record + 0x3C) + 0x2000000;
  random = rand() & 0xF;
  g_battle_work[9] = random + 1;
  g_battle_work[0xB] = 0;
  g_battle_work[2]++;
  claimed = reserveFirstFreeRecordBmagicMagic06303_801F0354();
  if (claimed != 0xFF) {
    slot = claimed * 0x20;
    /* The original reads the cell once for the pointer-word store and once more
     * for the trailing byte 0xB increment, and issues that second read before
     * the store; the two locals keep each read at its own site. */
    claims = g_battle_work;
    magic06303RecordFlags[slot + 1] = 1;
    count = g_battle_work;
    *(u8 **)(magic06303RecordFlags + slot + 0x1C) = claims;
    count[0xB]++;
  }
}
