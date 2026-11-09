#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell at 0x1F800044, bound by this target's own
 * map row g_battle_work. The original re-materialises the cell with its own
 * lui/lw pair before frame creation for the countdown, again for the byte 0xB
 * counter reset, twice inside each claim - once for the pointer word the claim
 * publishes and once for the spawn counter it advances - and once more after
 * the loop for the work byte 2 advance, because a store through the loaded
 * pointer may republish it. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Base of the 48-record pool this overlay keeps in its own image at 0x801F07F8,
 * right after the fifteen-word handler-pointer run at 0x801F07BC, with the same
 * 0x78-byte stride reserveFirstFreeRecordBmagicMagic09203_801F0758 scans. The
 * original reaches each field as `lui $at,%hi(base)` / `addu $at,$at,index*0x78`
 * / `sb|sw %lo(base + field)($at)`, so the fields 0x0, 0x1, 0x2, 0xB and word
 * 0x74 all stay flat addresses inside this one base. Splat binds the address
 * through this target's generated undefined_syms_auto.txt. */
extern u8 D_801F07F8[]; /* @source 0x801F07F8 @kind data */

/* Claims the first record of that pool whose flags byte has bit 0 clear, sets
 * the bit and returns the index, or returns 0xFF when all 48 records are taken.
 */
u8 reserveFirstFreeRecordBmagicMagic09203_801F0758(void);

/* Main-executable helper at 0x8015D71C. The handler family passes the selector
 * that this overlay wants the shared code to run; here it is 0x100 on the same
 * slot the sibling handlers reach with 0x101/0x102/0x103. Splat binds the
 * address through this target's generated undefined_syms_auto.txt. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C */

/* @source 0x801EF0CC
 * @behavior Entry 0 of the fifteen non-zero words of the in-image handler-
 *   pointer run based at 0x801F07BC, the word at 0x801F07C0 that the byte-2
 *   dispatcher at 0x801EF044 indexes with scratchpad work byte 2 scaled by
 *   four, so no jal in this payload names the 0x124-byte span. Every entry
 *   decrements the work record's byte 0x9 with a byte-width read-modify-write
 *   (lbu / addiu -1 / sb) and tests the stored value masked to a byte, so the
 *   body below runs only on the tick that countdown reaches zero.
 *   On that tick the work record's byte 0xB is cleared to zero and then, in a
 *   three-iteration loop, the body claims a free record of the pool at
 *   0x801F07F8 through reserveFirstFreeRecordBmagicMagic09203_801F0758 and
 *   skips the iteration when that claim returns 0xFF. The returned index is
 *   scaled by the 0x78-byte record stride and the record is seeded: byte 0x1
 *   receives the unsigned comparison `slot != 0` (so 0x0 for the first claim
 *   and 0x1 for the second and third), byte 0x2 receives zero, byte 0xB
 *   receives the loop ordinal itself, and the flags byte at 0x0 is or-ed with
 *   0x41 on top of the in-use bit the claim already set.
 *   The claim then reads the cell twice: the first read is the pointer word
 *   published at record offset 0x74, and the second read, issued before that
 *   store, advances the work record's byte 0xB, which counts the records this
 *   tick published. Both reads are separate cell loads in the original.
 *   After the loop the original calls the main-executable helper
 *   func_8015D71C with the constant 0x100 and then re-reads the cell once more
 *   to increment work byte 0x2, which advances this overlay's work byte 2
 *   handler sequence.
 *   Frame 0x18 keeps $ra at 0x18, $s1 at 0x14 - the hoisted 0xFF claim failure
 *   value - and $s0 at 0x10, the loop ordinal; the countdown's cell load is
 *   issued before frame creation. The span takes no arguments and returns
 *   nothing through the jr $ra at 0x801EF1E8; work bytes 0x2, 0x9 and 0xB and
 *   the record fields 0x0, 0x1, 0x2, 0xB and word 0x74 stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenClaimThreePoolRecordsSeedingSlotOrdinalAndWorkPointerInvokeHelperAt100AndAdvanceWorkByte2WhenZeroBmagicMagic09203_801EF0CC(
    void) {
  u8 slot;
  u8 index;
  u32 offset;
  u8 *work;
  u8 *counter;

  if (--g_battle_work[0x09] == 0) {
    g_battle_work[0x0B] = 0;
    for (slot = 0; slot < 3; slot++) {
      index = reserveFirstFreeRecordBmagicMagic09203_801F0758();
      if (index == 0xFF) {
        continue;
      }
      offset = index * 0x78;
      work = g_battle_work;
      D_801F07F8[offset + 0x01] = slot != 0;
      D_801F07F8[offset + 0x02] = 0;
      D_801F07F8[offset + 0x0B] = slot;
      D_801F07F8[offset] |= 0x41;
      counter = g_battle_work;
      *(u32 *)&D_801F07F8[offset + 0x74] = (u32)work;
      counter[0x0B]++;
    }
    func_8015D71C(0x100);
    g_battle_work[0x02]++;
  }
}
