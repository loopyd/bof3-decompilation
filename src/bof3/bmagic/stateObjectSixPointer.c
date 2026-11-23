#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044. This target's map binds no
 * name there, so the cell is reached through its address-anchored name. Every
 * access group below re-reads it, because a store through the loaded pointer may
 * alias the cell itself, and each read is bound to its own local: one local
 * carried from the guard group into the body keeps a second register and needs a
 * copy of the pointer before the func_801DD350 call, which measured 73 of 83
 * instructions and twelve surplus bytes against this span. */
extern u8 *D_1F800044; /* @source 0x1F800044 @kind unknown */

/* Shared battle record pointer in main RAM, outside this overlay's image. Splat
 * binds the address through this target's generated undefined_syms_auto.txt row
 * D_8014638C = 0x8014638C, so no target-map row is added. The sibling lift
 * decrementSeedHeaderBmagic
 * reads the same record through this cell: byte 8 as the stamp it copies into
 * the object's byte 8 and the words at 0x34, 0x38 and 0x3C as the record words
 * it accumulates into the object. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind unknown */

/* Byte 1, byte 9, byte 0xB and word 0x74 of the 0x78-byte-stride action-slot
 * store based at 0x801EC330 in the shared main executable. The original reaches
 * each field through its own flat symbol - lui $at,%hi(SYM), addu $at,$at,index
 * * 0x78, store %lo(SYM)($at) - with the scaled index shared by the four
 * stores, so every addressed field keeps its own address-anchored name instead
 * of a shared record base, exactly as the exact sibling
 * callHeadStampHalfwords
 * declares bytes 1 and word 0x74 of the same store. All four addresses are
 * bound by this target's generated undefined_syms_auto.txt rows. */
extern u8 D_801EC331[]; /* @source 0x801EC331 @kind data */
extern u8 D_801EC339[]; /* @source 0x801EC339 @kind data */
extern u8 D_801EC33B[]; /* @source 0x801EC33B @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Motion setup helper of the main executable, called with this overlay's work
 * object and returning nothing; the same prototype the sibling lift
 * decrementSeedHeaderBmagic
 * declares. */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 @kind function */

/* Shared battle slot allocator of the main executable at 0x801E590C, outside
 * this overlay's image; the exact sibling
 * callHeadStampHalfwords
 * declares the same two-argument prototype and passes the same mode pair. */
extern u32 func_801E590C(u32 mode0, u32 mode1); /* @source 0x801E590C @kind function */

/* Helper of the main executable at 0x8015D71C, outside this overlay's image,
 * called here with the single immediate 0x100. */
extern void func_8015D71C(s32 arg0); /* @source 0x8015D71C @kind function */

/* @source 0x801EEE94
 * @behavior Entry 1 of the four-entry work byte 1 handler run at 0x801EEC04
 *           (0x801EEC90, 0x801EEE94, 0x801EEFE0, 0x801EF060), a run that lives
 *           in the overlay's own table words and that the exact sibling
 *           dispatcher dispatchWorkByte1HandlerBmagicMagic15403_801EEC24 copies
 *           into its 0x28-byte frame and invokes with the work record's
 *           unsigned byte 1 scaled by four, so no jal in this payload names this
 *           address: an overlay dispatch entry, not a called function. It reads
 *           the scratchpad work-object pointer cell at 0x1F800044 before its
 *           frame exists and tests the object's byte 0xB; a non-zero result
 *           returns through the shared epilogue, so the seeding below runs once
 *           per work object. At zero it copies byte 0x8 of the shared battle
 *           record at 0x8014638C into the object's byte 0x8, clears the delta
 *           word at 0xC, seeds the word at 0x10 with 0xFFFE8000 (-0x18000 in
 *           16.16) and hands the object to the main-executable motion setup
 *           func_801DD350. It then reloads both pointers and, because that call
 *           may have written the two deltas, reads them back while accumulating
 *           the record's words 0x34 and 0x38 into the same offsets of the
 *           object and copying the record's word 0x3C into the object's word
 *           0x3C. The call func_8015D71C(0x100) follows with that last store in
 *           its delay slot. The claim loop then runs six times: it claims a
 *           queued slot through func_801E590C(1, 0x52), uses the low byte of the
 *           result as the index into the 0x78-byte-stride store at 0x801EC330,
 *           and writes byte +0x01 = 0, byte +0x0B = the loop index, byte +0x09 =
 *           1 + three times that index and word +0x74 = the scratch work
 *           pointer, incrementing the object's byte 0xB once per claim. It
 *           finishes by reading the pointer cell again and incrementing work
 *           byte 1, advancing the work byte 1 state to run entry 2
 *           (0x801EEFE0). It takes no arguments, returns nothing and writes no
 *           other state; the 0x20-byte frame keeps $ra at 0x1C and $s0, $s1 and
 *           $s2 at 0x10, 0x14 and 0x18.
 *
 *           No byte-identical sibling exists: a payload scan of all 398
 *           extracted binaries found this 0x14C-byte span in this target only,
 *           so the clone lever is declined. Statement order, operand forms,
 *           offset-local spelling and declaration style follow the sibling lifts
 *           decrementSeedHeaderBmagic,
 *           which carries the same guard shape, the same byte 0x8 copy, the same
 *           delta-word seed before func_801DD350, the same two pointer reloads
 *           and the same accumulated record triplet, and the exact lift at
 *           0x801EED1C of the magic10503 overlay, whose claim loop over the same
 *           0x801EC330 store carries the same low-byte scaled index, byte store
 *           order, published work pointer and per-claim byte 0xB increment. Both
 *           are shape evidence and not clone seeds. This span's own bytes
 *           supplied the guard byte 0xB, the seed literals 0 and 0xFFFE8000, the
 *           six-iteration bound, the slot mode 0x52, the byte 0x09 step of three
 *           from one, the helper immediate 0x100 and the closing work byte 1
 *           increment.
 *
 *           Representation notes from the probes before the exact gate, all
 *           clean C with the owning compiler and flags unchanged. One local for
 *           every read of the pointer cell scored 73 of 83: the guard read took
 *           $a1, the reload re-used it, and the func_801DD350 call needed a move
 *           out of $a1 into $a0, three instructions over this span.
 *           Read-local splitting with a byte loop index scored 78 of 83 and
 *           kept the move. Splitting the pre-call and post-call reads into
 *           separate locals scored 81 of 84. The unsigned loop index removes the
 *           andi/sltiu pair a byte index needs before its bound test, and with
 *           the index increment in the for clause and the byte 0x09 ramp in the
 *           loop body the increment lands in the load delay slot of the count
 *           read, which closed the last instruction and reached the exact match.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateObjectSixPointer(
    void) {
  u8 *record;
  u8 *work;
  u8 *pos;
  u8 *acc;
  u8 *slot;
  u8 *count;
  u32 index;
  u32 step;
  u32 offset;

  work = D_1F800044;
  if (work[0x0B] != 0) {
    return;
  }
  record = D_8014638C;
  work[8] = record[8];
  pos = D_1F800044;
  *(u32 *)(pos + 0xC) = 0;
  *(u32 *)(pos + 0x10) = -0x18000;
  func_801DD350(pos);
  acc = D_1F800044;
  *(s32 *)(acc + 0x34) = *(s32 *)(record + 0x34) + *(s32 *)(acc + 0xC);
  *(s32 *)(acc + 0x38) = *(s32 *)(record + 0x38) + *(s32 *)(acc + 0x10);
  *(s32 *)(acc + 0x3C) = *(s32 *)(record + 0x3C);
  func_8015D71C(0x100);
  for (index = 0, step = 1; index < 6; index++) {
    offset = (u8)func_801E590C(1, 0x52) * 0x78;
    slot = D_1F800044;
    D_801EC331[offset] = 0;
    D_801EC33B[offset] = index;
    D_801EC339[offset] = step;
    count = D_1F800044;
    *(u32 *)&D_801EC3A4[offset] = (u32)slot;
    count[0x0B] = count[0x0B] + 1;
    step += 3;
  }
  D_1F800044[1]++;
}
