#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell, bound through the target symbol table so
 * each read expands to the split-address `lui %hi` plus folded `%lo`
 * displacement the original shows. The original re-reads the cell with its own
 * address load for the opening step-word group, for the 0x34/0x38 coordinate
 * pair passed to the distance routine, for the comparison group that also
 * serves the four release stores, and once more at the end of the release arm
 * for the handler-byte check.
 */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */
/* Pointer cell of the shared work record whose halfword at 0x122 this step
 * consumes two units of. */
extern u8* D_80146250; /* @source 0x80146250 @kind unknown */
/* Shared world-flag pair: bit 3 of either halfword gates the hand-off of the
 * work record's signed 0x3E halfword. */
extern u16 D_80146258; /* @source 0x80146258 @kind data */
extern u16 D_8014625A; /* @source 0x8014625A @kind data */

/* Shared resident mover: advances the work record by the step word this step
 * leaves in the argument register and consumes no result. */
void func_801B0DFC(void);
/* Shared resident routine taking the coordinate pair at 0x34/0x38 and
 * returning its distance, which its callers narrow to a signed halfword. */
s32 func_80154F28(s32 x, s32 y);
/* Shared resident routine taking one value; its body lies outside every overlay
 * payload, so only the one-argument call form is established here. */
void func_80154FD8(u32 arg0);

/* @source 0x801F3354
 * @behavior Second entry of this overlay's three-entry work-byte-4 handler table
 *           at 0x801F3898. It adds the scratch work record's 0x20 step word to
 *           its 0x14 motion word, keeps that step word live for the shared mover
 *           func_801B0DFC called immediately after, and then queries the shared
 *           distance routine func_80154F28 with the record's 0x34/0x38
 *           coordinate pair. When the record's signed halfword at 0x3E is below
 *           the distance narrowed to a signed halfword, the record is released:
 *           the raw call result is stored into the 0x3E halfword, the 0x14 and
 *           0x20 words are zeroed and 2 is written into the handler byte at 0x04,
 *           which hands the next frame to the third table entry. Both paths then
 *           continue into the shared tail, which reads the record's byte at 0x05
 *           and bit 3 of the shared world-flag pair 0x80146258/0x8014625A: while
 *           that byte and that bit are both zero it hands the record's signed
 *           0x3E halfword to func_80154FD8, and it always consumes two units of
 *           the halfword at 0x122 of the record published through the global
 *           pointer cell 0x80146250. Takes no arguments and returns nothing; the
 *           value left in $v0 is the consumed countdown, which no caller reads.
 * @status exact
 * @match 100.00
 * @residual none
 */
/*
 * The opening step-word update reads the cell twice in one expression, so the
 * single load the original keeps is the compiler's own CSE and the step word
 * stays in the register the add leaves it in. The narrowed distance is its own
 * signed-halfword local: the original copies the raw call result into a second
 * register, narrows the call-result register itself into that local and stores
 * the raw copy into the 0x3E halfword afterwards, which is the spelling that
 * measured exact at world04/area172/13 0x801F44C4 for the same register
 * split; an inline cast at the comparison narrowed the copy instead and cost
 * that one sll source register. The release stores name the cell inline
 * rather than a record local, because the original reuses the pointer the
 * comparison group already loaded and re-reads the cell only once at the end of
 * that arm, for the handler-byte check that follows both paths. The release
 * statement order and the closing countdown follow the body that measured exact
 * at world03/area145/14 0x801F3F18, which ran the same opening pair and the same
 * distance test against the 0x3E halfword but ended in an if/else.
 */
void advanceWorkMotionAndSelectHandler2PastWork3eThenHandHalfword3eToSharedUnlessByte5OrFlag8SetAndConsumeSharedCountdownWorld01Area05613_801F3354(
    void) {
  s32 result;
  s16 distance;

  *(s32*)(D_1F800044 + 0x14) += *(s32*)(D_1F800044 + 0x20);
  func_801B0DFC();
  result =
      func_80154F28(*(s32*)(D_1F800044 + 0x34), *(s32*)(D_1F800044 + 0x38));
  distance = (s16)result;
  if (*(s16*)(D_1F800044 + 0x3e) < distance) {
    *(s16*)(D_1F800044 + 0x3e) = result;
    *(s32*)(D_1F800044 + 0x14) = 0;
    *(s32*)(D_1F800044 + 0x20) = 0;
    D_1F800044[4] = 2;
  }
  if (D_1F800044[5] == 0 && ((D_80146258 | D_8014625A) & 8u) == 0) {
    func_80154FD8(*(s16*)(D_1F800044 + 0x3e));
  }
  *(u16*)(D_80146250 + 0x122) -= 2;
}
