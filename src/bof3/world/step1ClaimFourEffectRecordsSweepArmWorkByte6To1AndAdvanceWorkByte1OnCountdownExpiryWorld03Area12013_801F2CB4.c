#include "bof3/bof3.h"

/* Scratchpad work-record pointer cell. The original materialises the cell
 * address with `lui` plus an offset load once for the countdown
 * read-modify-write and the mode-byte store, and again for the re-arm and the
 * step advance; the two loads land in different registers, so the source keeps
 * one pointer local per group.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Alternating-direction claim pass of this target at 0x801F3630, which its own
 * claimed source defines; only its prototype is declared here. It takes the
 * first free entry of the shared 0x1C-byte-stride effect table by writing 1
 * into that entry's lead byte and returns nothing, so the four consecutive
 * calls below read no result. */
void claimFreeEffectRecordWithAlternatingDirectionWorld03Area12013_801F3630(void);

/* Per-frame sweep of this target at 0x801F36B0, which its own claimed source
 * defines. It walks the live entries of that same effect table and returns
 * whether any entry was live; this handler reads no result. */
s32 advanceWorkRecordStatesAndRenderActiveWorld03Area12013_801F36B0(void);

/* @source 0x801F2CB4
 * @behavior Step 1 of the run this overlay dispatches through the handler
 *           table at 0x801F43C0, whose second pointer word names this body,
 *           and the table the exact dispatcher at 0x801F2C18 indexes with
 *           work-record byte 0x01. The body first runs four consecutive claims
 *           through the target-local claim pass at 0x801F3630 and ignores
 *           every result, then runs the target-local per-frame effect sweep at
 *           0x801F36B0 and ignores its result too. It then reads the work
 *           record published through the scratchpad pointer cell 0x1F800044
 *           once, decrements that record's unsigned halfword at +0x32, stores
 *           the decremented value back and tests it, so every frame but the
 *           last one returns here. On the frame the halfword reaches zero the
 *           handler stores 1 into the record byte at +0x06, the record byte
 *           the effect sweep reads as the state step it adds, reads the
 *           pointer cell again, refills the halfword at +0x32 with 0x3C and
 *           advances the record byte at +0x01 by one, which sends the next
 *           frame to entry 2 of the same table, the handler at 0x801F2D40.
 *           Takes no arguments, returns nothing, and its 0x18-byte frame holds
 *           only the saved return address at 0x10. The roles of the record
 *           halfword at +0x32 and of the record byte at +0x06 stay described
 *           by offset.
 *
 * The countdown uses unsigned halfword memory accesses and a signed 16-bit
 * local for the decremented value. The zero test uses that truncated value.
 * @status exact
 * @match 100.00
 * @residual none
 */
void step1ClaimFourEffectRecordsSweepArmWorkByte6To1AndAdvanceWorkByte1OnCountdownExpiryWorld03Area12013_801F2CB4(
    void) {
  u8* work;
  u8* stage;
  s16 remain;

  claimFreeEffectRecordWithAlternatingDirectionWorld03Area12013_801F3630();
  claimFreeEffectRecordWithAlternatingDirectionWorld03Area12013_801F3630();
  claimFreeEffectRecordWithAlternatingDirectionWorld03Area12013_801F3630();
  claimFreeEffectRecordWithAlternatingDirectionWorld03Area12013_801F3630();
  advanceWorkRecordStatesAndRenderActiveWorld03Area12013_801F36B0();

  work = D_1F800044;
  remain = *(u16*)(work + 0x32) - 1;
  *(u16*)(work + 0x32) = remain;
  if (remain == 0) {
    work[6] = 1;
    stage = D_1F800044;
    *(u16*)(stage + 0x32) = 0x3C;
    stage[1]++;
  }
}
