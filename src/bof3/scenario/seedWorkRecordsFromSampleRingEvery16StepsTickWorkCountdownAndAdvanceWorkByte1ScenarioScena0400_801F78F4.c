#include "bof3/bof3.h"

u8* findFreeWorkRecordByte25ScenarioScena0400_801F7FF8(void);
void stepDrawIntegrateMotionClampHeightAndDecayActiveWorkRecordsScenarioScena0400_801F7EC0(
    void);

/* @source 0x80147A8C @kind data
 * Resident position triplet in main RAM; this overlay copies these three words
 * into the sample ring base at 0x800E5C00 as a position pair plus a third word.
 */
extern s32 D_80147A8C;
/* @source 0x80147A90 @kind data */
extern s32 D_80147A90;
/* @source 0x80147A94 @kind data */
extern s32 D_80147A94;

/* @source 0x80178818 @kind unknown
 * Shared in-place vector helper: it takes one work entry's three position
 * words twice and rewrites them where they stand.
 */
extern void func_80178818(void* arg0, void* arg1);

typedef struct {
  s32 word0;
  s32 word1;
  s32 word2;
  s32 word3;
} ScenarioScena0400SampleRingBase;

/* @source 0x801F78F4
 * @behavior Entry 7 of this overlay's nine-word work-byte-1 handler run at
 *           0x801FA180 ({0x801F6D44, 0x801F6DD4, 0x801F6EA4, 0x801F6F2C,
 *           0x801F6FF8, 0x801F7064, 0x801F77EC, 0x801F78F4, 0x801F7A64}), so
 *           the dispatcher dispatchWorkByte1HandlerScenarioScena0400_801F6D00
 *           reaches it only through its indirect call, and the second
 *           dispatcher at 0x801F77A8 reaches it as index 1 of the three-word
 *           tail of that run, which it indexes from 0x801FA198.
 *           It loads the work-object pointer from the scratchpad cell
 *           0x1F800044 and reads that object's word at +0x0C, the countdown the
 *           previous entry armed with 0x200. Only when that word reads zero in
 *           its low four bits, that is once every sixteen steps of that
 *           countdown, does it run its seeding block, so the eight records are
 *           republished every sixteenth frame of the 512-step tail.
 *           The seeding block walks the eight 0x10-byte sample records of the
 *           ring based at 0x800E5C00, which entry 6 at 0x801F77EC fills with
 *           one sampled point per direction around the resident position. Each
 *           turn of the walk claims one free work record through the 0x80-entry
 *           work-table scan findFreeWorkRecordByte25ScenarioScena0400_801F7FF8
 *           (jal at 0x801F7934, delay slot a nop) and leaves the record
 *           untouched when the scan reports none. For a claimed record it
 *           copies the ring base words at +0x00, +0x04, +0x08 and +0x0C into
 *           the record's words at +0x00, +0x04, +0x08 and +0x0C, then stores at
 *           +0x10, +0x14 and +0x18 the difference between the sample record's
 *           word at +0x10, +0x14 and +0x18 and the matching ring base word at
 *           +0x00, +0x04 and +0x08, arithmetic shifted right by four, so each
 *           record carries its sample point as an offset from the ring origin
 *           in sixteenths of a step. That offset triple is handed to the shared
 *           vector helper func_80178818 in place (jal at 0x801F79BC, the third
 *           offset store filling its delay slot), and the record then takes
 *           0x1000 in its word at +0x20 and the claim marker 0x80 in bytes
 *           0x24 and 0x25, the byte the free-record scan tests.
 *           After the walk, and also on the sixteen-step guard's skip arm, it
 *           re-seeds the ring base from the resident position triplet: +0x00
 *           takes D_80147A8C, +0x04 takes D_80147A90 and +0x08 takes D_80147A94
 *           shifted right by eight arithmetic, with the last store filling the
 *           delay slot of the record pass call. It then runs the overlay's
 *           record pass
 *           stepDrawIntegrateMotionClampHeightAndDecayActiveWorkRecordsScenarioScena0400_801F7EC0
 *           (0x801F7EC0, no arguments, no result), re-reads
 *           the work-object cell because the pass may rewrite it, decrements
 *           its countdown word at +0x0C by one, storing it on both arms because
 *           the store sits in the branch delay slot, and, only when the stored
 *           value reaches zero, increments work byte 1 so the tail dispatcher
 *           next selects 0x801F7A64.
 *           It takes no arguments, consumes no result, and its 0x38-byte frame
 *           keeps $s0, $s1, $s2, $s3 and $ra across the two calls. None of the
 *           frame bytes below 0x20 is read or written, so the original
 *           translation unit carried a declaration that emitted no code; the
 *           frame_reserve array restores the missing 0x10 bytes and nothing
 *           else.
 *
 * Name evidence: the only image word holding 0x801F78F4 is 0x801FA19C, word 7 of
 *           the run at 0x801FA180 and index 1 of the three-word tail the second
 *           dispatcher indexes from 0x801FA198, so no jal names this body and
 *           the dispatch shape is the naming route. The seeding role comes from
 *           the claim scan and the stored words, which are the same claim
 *           marker and the same 0x800E5C00 ring the entry 6 at 0x801F77EC
 *           fills, the sixteen-step gate from the low-nibble test on the
 *           countdown word, and the work-byte advance from the byte this body
 *           increments.
 *
 * Shape evidence: statement order, operand forms and the `*(s32*)(record + N)`
 *           spelling follow the exact in-target siblings that claim a work
 *           record from the same 0x800E4800 table
 *           (claimSeedWorkRecordEvery8FramesAndAdvanceWorkByte1AtProgress3ScenarioScena0400_801F7B00,
 *           including the 0x24 / 0x20 / 0x25 store order) and the ring-base
 *           seed group follows the exact entry 6 at 0x801F77EC. The decrement
 *           whose store lands in the guard branch delay slot follows the exact
 *           in-target step handler
 *           stepActiveWorkRecordsTickWorkCountdownAndResetWorkStateOnZeroScenarioScena0400_801F7CA8,
 *           which ticks the same word. Every offset, bound, shift count and
 *           stored literal comes from this span's own bytes. The clone lever is
 *           declined on a masked payload scan: with the three jal immediate
 *           words masked, these 0x170 bytes occur at no other offset of the 396
 *           payloads under out/binaries, and the one hit at this span's own
 *           offset is the scan's self-match.
 *
 * Decisive shapes: the ring header reaches the record as one 0x10-byte
 *           aggregate assignment, which is what puts the four loads ahead of
 *           the four stores; the per-word spelling instead emits load/store
 *           pairs with load-delay nops and pulls the sample-index arithmetic
 *           into the null-record branch delay slot. The original frame reserves
 *           0x10 bytes beyond the five saved registers, which the declared
 *           frame_reserve array reproduces. The head guard and the tail each
 *           load the scratch work cell into their own local, because one local
 *           carrying both reload points is a single pseudo and lands whole in
 *           $v1, which moves the head pointer out of the $v0 the original uses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkRecordsFromSampleRingEvery16StepsTickWorkCountdownAndAdvanceWorkByte1ScenarioScena0400_801F78F4(
    void) {
  u8* base;
  u8* sample;
  u8* record;
  u8* work;
  u8* next;
  s32 count;
  u8 frame_reserve[0x10]; /* size-only frame reservation */
  u8 i;

  work = SPAD_PTR_SLOT(u8, 0x44u);
  base = PSX_PTR(u8, 0x800E5C00u);
  if ((*(s32*)(work + 0xC) & 0xF) == 0) {
    i = 0u;
    do {
      record = findFreeWorkRecordByte25ScenarioScena0400_801F7FF8();
      if (record != 0) {
        *(ScenarioScena0400SampleRingBase*)record =
            *(ScenarioScena0400SampleRingBase*)base;
        sample = base + (i << 4);
        *(s32*)(record + 0x10) =
            (*(s32*)(sample + 0x10) - *(s32*)(base + 0x00)) >> 4;
        *(s32*)(record + 0x14) =
            (*(s32*)(sample + 0x14) - *(s32*)(base + 0x04)) >> 4;
        *(s32*)(record + 0x18) =
            (*(s32*)(sample + 0x18) - *(s32*)(base + 0x08)) >> 4;
        func_80178818(record + 0x10, record + 0x10);
        record[0x24] = 0x80;
        *(s32*)(record + 0x20) = 0x1000;
        record[0x25] = 0x80;
      }
      i += 1u;
    } while (i < 8u);
  }
  *(s32*)(base + 0x00) = D_80147A8C;
  *(s32*)(base + 0x04) = D_80147A90;
  *(s32*)(base + 0x08) = D_80147A94 >> 8;
  stepDrawIntegrateMotionClampHeightAndDecayActiveWorkRecordsScenarioScena0400_801F7EC0();
  next = SPAD_PTR_SLOT(u8, 0x44u);
  count = *(s32*)(next + 0xC) - 1;
  *(s32*)(next + 0xC) = count;
  if (count == 0) {
    next[1]++;
  }
}
