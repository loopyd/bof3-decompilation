#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell at 0x1F800044, bound by this target's map
 * row g_battle_work. The original re-materializes the cell with its own
 * `lui` / `lw` pair at each of the three reads - the one whose pointer is
 * published into the claimed slot, the one the byte 0xB clear goes through and
 * the one the byte 0x01 increment goes through - because the stores that follow
 * each read may republish the pointer. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Current-record pointer cell at 0x8014638C in main RAM, outside this overlay
 * image and bound through this target's generated undefined_syms_auto.txt. The
 * original loads it once, ahead of the slot-allocator call, and keeps it in a
 * saved register across that call, so the record it names is the one the
 * allocator saw. The bmagic siblings that read the same cell declare it with
 * this spelling. */
extern u8 *D_8014638C; /* @source 0x8014638C @kind data */

/* Action-slot pool at 0x801EC330 in main RAM, outside this overlay image: the
 * 0x78-byte stride pool the bmagic slot lifts document. The original reaches
 * each field through its own flat symbol - `lui $at,%hi(SYM)`, `addu $at,$at,
 * index*0x78`, access `%lo(SYM)($at)` - with the scaled index in one register
 * shared by all five accesses, so every addressed field keeps its own
 * address-anchored name: the record byte at +0x08, the record words at +0x34,
 * +0x38 and +0x3C and the work-object pointer word at +0x74. */
extern u8 D_801EC338[]; /* @source 0x801EC338 @kind data */
extern u8 D_801EC364[]; /* @source 0x801EC364 @kind data */
extern u8 D_801EC368[]; /* @source 0x801EC368 @kind data */
extern u8 D_801EC36C[]; /* @source 0x801EC36C @kind data */
extern u8 D_801EC3A4[]; /* @source 0x801EC3A4 @kind data */

/* Slot allocator of the battle image at 0x801E590C, outside this overlay and
 * bound through this target's generated undefined_funcs_auto.txt. It returns the
 * index of the claimed action slot, which this body truncates to a byte before
 * scaling it by the 0x78-byte stride. */
extern u32 func_801E590C(u32 arg0, u32 arg1); /* @source 0x801E590C */

/* @source 0x801EEC5C
 * @behavior Begin step of this overlay's two-entry work-byte-1 handler pair. It
 *           loads the current-record pointer from 0x8014638C before the call and
 *           claims one action slot through the allocator at 0x801E590C with the
 *           arguments 1 and 0x1A, truncates the returned index to a byte and
 *           scales it by the 0x78-byte slot stride; the index is not checked for
 *           failure. It publishes a work-object pointer read from the
 *           scratchpad cell 0x1F800044 into that slot's word +0x74, copies the
 *           record's byte +0x08 into the slot's byte +0x08, takes a second read
 *           of the same cell and copies the record's words +0x34, +0x38 and
 *           +0x3C into the slot's words +0x34, +0x38 and +0x3C. That second
 *           pointer then has its byte 0xB cleared, and a third read of the cell
 *           carries the byte 0x01 increment. The 0x18-byte
 *           frame keeps $s0 and $ra only, the record cell is read before the
 *           frame's $ra save and every branch slot stays empty. The body takes
 *           no arguments and returns nothing. No direct branch and no aligned
 *           pointer word in the payload names 0x801EEC5C; the two-word handler
 *           table the dispatcher at 0x801EEC04 builds is its only in-payload
 *           reference, and the overlay loader reaches it through that
 *           dispatcher. Statement order, operand forms, offset spelling and
 *           declaration style follow the exact bmagic siblings
 *           queueTwoLinkedSlotsCopyRecordTripletsThenInvokeHelperAt100AndAdvanceWorkByte1BmagicMagic07803_801EECFC
 *           (the same record cell saved before the allocator call, the same word
 *           triplet copied from that record into the slot bands, and the same
 *           closing byte 0xB clear followed by the byte 0x01 increment, both on
 *           the named cell) and
 *           setWorkByte9To18ClaimTwoSlotsCopyHalfwordTableWithBit15SetAndWhenActingRecordByteF0Is29ClaimSlotCopyWorkHeadBmagicMagic13703_801EEC7C
 *           (the work-pointer publish into word +0x74 followed by the record's
 *           byte +0x08 store). This body's own literals 1 and 0x1A, its record
 *           cell and its band offsets are written from the original bytes. No
 *           byte-identical copy of this 0xC0-byte span exists: the span occurs
 *           in none of the 398 emi payloads of this repository's binaries.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Representation notes from the shape probes measured before this body
 * matched: leaving the byte 0xB clear as a literal `g_battle_work[0xB] = 0` at
 * the tail made the compiler materialise the cell there and pad the load delay
 * with a `nop` (45/48 instructions, 196-byte body), while reassigning one
 * pointer local for both the published and the cleared read kept the 192-byte
 * size but put the published pointer in $a0 instead of $v0 (45/48). Reading the
 * cell into the second local after the slot byte +0x08 store and before the
 * three record-word stores, as the body below does, is the measured exact form.
 */
void queueActionPointerTriplet(
    void) {
  u8 *record;
  u8 *work;
  u8 *slotWork;
  u32 offset;

  record = D_8014638C;
  offset = (u8)func_801E590C(1u, 0x1Au) * 0x78u;
  work = g_battle_work;
  *(u8 **)&D_801EC3A4[offset] = work;
  D_801EC338[offset] = record[8];
  /* Second read of the pointer cell: the record-word stores below may publish
   * a different work object, so the byte 0xB clear keeps the pointer this read
   * returned. */
  slotWork = g_battle_work;
  *(u32 *)&D_801EC364[offset] = *(u32 *)(record + 0x34);
  *(u32 *)&D_801EC368[offset] = *(u32 *)(record + 0x38);
  *(u32 *)&D_801EC36C[offset] = *(u32 *)(record + 0x3C);
  slotWork[0xB] = 0;
  g_battle_work[1]++;
}
