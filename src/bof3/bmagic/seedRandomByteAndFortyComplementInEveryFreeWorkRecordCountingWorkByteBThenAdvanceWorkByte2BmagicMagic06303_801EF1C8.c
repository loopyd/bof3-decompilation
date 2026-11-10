#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * as g_battle_work, the name the exact bmagic lifts of this family use for the
 * same main-executable cell. The original loads the cell once before the frame
 * is created for the guarded entry test, and re-reads it at each site that
 * follows a call or a store through a pointer it loaded. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Record pool at 0x801F0454: 0x60 records of stride 0x20 whose byte 0 carries
 * the in-use bit reserveFirstFreeRecordBmagicMagic06303_801F0354 sets and whose
 * bytes 0, 1 and 2 the reset at 0x801EECF8 clears. This body fills the claimed
 * record byte 1, byte 4, bytes 8 and 0x0A and the pointer word at +0x1C, and
 * the original reaches each of them as `lui $at,%hi(ADDR)` /
 * `addu $at,$at,index*0x20` / `sb|sw %lo(ADDR)($at)` over the flat addresses
 * 0x801F0455, 0x801F0458, 0x801F045C, 0x801F045E and 0x801F0470, so each
 * addressed field stays an offset of this one named base. */
extern u8 magic06303RecordFlags[]; /* @source 0x801F0454 @kind data */

/* Claimed-record allocator of this overlay: returns the index of the first pool
 * record whose flags byte has bit 0 clear, sets that bit, or returns 0xFF once
 * all 0x60 are occupied, which is the value this body tests. It sits behind a
 * jal in the original, so it keeps an extern prototype. */
extern u8 reserveFirstFreeRecordBmagicMagic06303_801F0354(void); /* @source 0x801F0354 */

/* PsyQ C runtime at 0x8017E3D4. Only the low four bits of the answer are used,
 * as the random seed byte of the pair this body writes. */
extern int rand(void);

/* Helper of the main executable at 0x8015D71C. The bmagic sibling
 * seedMotionTintAndChannelFieldsThenAdvanceWorkByte2BmagicMagic00800_801F05FC
 * declares the same one-argument prototype and passes a constant selector; this
 * body passes 0x101 and then 0x102. */
void func_8015D71C(s32 arg0);

/* @source 0x801EF1C8
 * @behavior Entry 3 of the five-word code pointer run at 0x801EEC04
 *           (0x801EEFE4, 0x801EF110, 0x801EF190, 0x801EF1C8, 0x801EF2E4), the
 *           table the overlay dispatcher at 0x801EEF74 copies into its own
 *           frame and indexes by work byte 2; no jal word of this payload names
 *           this address, so it is reached through that run alone.
 *           It reads the scratchpad work-object pointer cell at 0x1F800044 once
 *           before its 0x20-byte frame is created and returns at once when work
 *           byte 0xB is not zero, so the seeding pass below runs only while that
 *           byte is clear. The pass tries to claim a pool record 0x5C times
 *           through reserveFirstFreeRecordBmagicMagic06303_801F0354, gives up on
 *           a 0xFF answer without leaving the loop, and for each claimed record
 *           clears byte 1, writes the pass counter itself into byte 4, stores
 *           the work-object pointer into the record word at offset 0x1C, then
 *           seeds this record's random byte pair: byte 0x0A takes one plus the
 *           low four bits of rand() added to the pass counter halved and masked
 *           with 0x70, and byte 8 takes 0x40 minus that value, so the two bytes
 *           always sum to 0x40 at byte width. Each claim then increments work
 *           byte 0xB through a fresh read of the cell, so that byte ends as the
 *           number of records this pass claimed, 0 once the pool was already
 *           full. After the pass it invokes the shared helper at 0x8015D71C
 *           with 0x101 and then with 0x102 and increments work byte 2, the
 *           handler selector the dispatcher reads. Takes no arguments, returns
 *           nothing and writes no other state; the roles of work bytes 0xB and
 *           2 beyond the claimed-record count and the selector their call sites
 *           show, of record bytes 1, 4, 8 and 0x0A, and of the two helper
 *           selectors stay unestablished, so those stay offset-anchored. Every
 *           cell read sits at its own site, and the four reloads split into two
 *           register groups: the guard and the record pointer store share one
 *           local and the claimed-record increment shares the other with the
 *           trailing advance of work byte 2. The frame keeps $ra at 0x18 with
 *           $s1 and $s0 at 0x14 and 0x10. The same-target exact sibling
 *           copySelectedBattlerRecordTripletPlus2000000SeedByte9RandomZeroByteB
 *           ClaimPoolRecordCountByteBAdvanceWorkByte2BmagicMagic06303_801EEFE4
 *           supplied the claim-and-stamp shape and the site-local cell reads,
 *           and reserveFirstFreeRecordBmagicMagic06303_801F0354 the byte-masked
 *           loop counter form; the byte pair, the pass counter store and the
 *           two fixed-selector calls come from this span's own bytes. The pool
 *           and cell addresses are this target's, so no byte-identical sibling
 *           exists and no clone was applied.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedRandomByteAndFortyComplementInEveryFreeWorkRecordCountingWorkByteBThenAdvanceWorkByte2BmagicMagic06303_801EF1C8(void) {
  u8 *work;
  u8 *count;
  u8 claimed;
  u8 index;
  s32 value;
  u32 slot;

  work = g_battle_work;
  if (work[0xB] != 0) {
    return;
  }
  for (index = 0; index < 0x5C; index++) {
    claimed = reserveFirstFreeRecordBmagicMagic06303_801F0354();
    if (claimed != 0xFF) {
      slot = claimed * 0x20;
      work = g_battle_work;
      magic06303RecordFlags[slot + 1] = 0;
      magic06303RecordFlags[slot + 4] = index;
      *(u8 **)(magic06303RecordFlags + slot + 0x1C) = work;
      value = ((index >> 1) & 0x70) + (rand() & 0xF) + 1;
      magic06303RecordFlags[slot + 0xA] = value;
      magic06303RecordFlags[slot + 8] = 0x40 - value;
      /* The original reads the cell once more after the pair stores. Its four
       * reloads land in two registers, so the body keeps two locals: this read
       * and the trailing work byte 2 advance share the second one, while the
       * guard and the record pointer store share the first. */
      count = g_battle_work;
      count[0xB]++;
    }
  }
  func_8015D71C(0x101);
  func_8015D71C(0x102);
  count = g_battle_work;
  count[2]++;
}
