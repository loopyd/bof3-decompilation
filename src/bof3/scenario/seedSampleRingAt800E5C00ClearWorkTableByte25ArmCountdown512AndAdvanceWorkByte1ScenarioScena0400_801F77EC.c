#include "bof3/bof3.h"

void clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC(void);

/* @source 0x8015477C @kind unknown
 * Shared fixed-point distance helper over the word pair passed as (x, z);
 * callers publish its result scaled by 0x10000.
 */
extern s16 func_8015477C(s32 arg0, s32 arg1);

/* @source 0x80147A8C @kind data
 * Resident position triplet in main RAM; the overlay sources copy these three
 * words into their work objects as a position pair plus a third word.
 */
extern s32 D_80147A8C;
/* @source 0x80147A90 @kind data */
extern s32 D_80147A90;
/* @source 0x80147A94 @kind data */
extern s32 D_80147A94;

/* @source 0x801FA1A4 @kind data
 * Eight 16.16 direction words that follow the nine-word handler run at
 * 0x801FA180: (1,0), (-1,0), (0,1), (0,-1), (1,1), (-1,1), (1,-1), (-1,-1).
 */
extern s32 D_801FA1A4[];
/* @source 0x801FA1C4 @kind data
 * The second half of that eight-pair direction table, 0x20 bytes into it.
 */
extern s32 D_801FA1C4[];

/* @source 0x801F77EC
 * @behavior Entry 6 of this overlay's nine-word work-byte-1 handler run at
 *           0x801FA180 ({0x801F6D44, 0x801F6DD4, 0x801F6EA4, 0x801F6F2C,
 *           0x801F6FF8, 0x801F7064, 0x801F77EC, 0x801F78F4, 0x801F7A64}), so
 *           the dispatcher dispatchWorkByte1HandlerScenarioScena0400_801F6D00
 *           reaches it only through its indirect call, and the second
 *           dispatcher at 0x801F77A8 reaches it as index 0 of the three-word
 *           tail of that run, which it indexes from 0x801FA198.
 *           It seeds the eight-record sample ring based at 0x800E5C00 from the
 *           resident position triplet: 0x80147A8C lands in the ring base at
 *           +0x00, 0x80147A90 at +0x04 and 0x80147A94 at +0x08 shifted right by
 *           eight arithmetic. Each of the eight 0x10-byte records then takes at
 *           +0x10 the ring base word at +0x00 plus the 16.16 offset word
 *           D_801FA1A4[i], at +0x14 the ring base word at +0x04 plus
 *           D_801FA1C4[i], and at +0x18 the shared distance helper
 *           func_8015477C answer for that pair, sign-extended from its halfword
 *           return and shifted left by eight (sll 16 / sra 8). The two offset
 *           tables are the eight unit directions (1,0), (-1,0), (0,1), (0,-1),
 *           (1,1), (-1,1) and the matching negatives, so the ring holds one
 *           sampled point per direction around the resident position. The loop
 *           runs eight times over a byte counter masked to 0xFF and tested
 *           below 8 unsigned; it keeps the 0x800E5C00 pointer in a callee-saved
 *           register for the whole walk, so only the first two base writes name
 *           absolute addresses, and it re-reads the two ring base words every
 *           iteration because the record stores may touch them.
 *           After the loop it calls the overlay's work-table clear helper
 *           clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC (jal at
 *           0x801F78AC, delay slot a nop), re-reads the work-object pointer
 *           from the scratchpad cell 0x1F800044, stores 0x200 in the work
 *           countdown word at +0x0C, and increments work byte 1 so the tail
 *           dispatcher next selects 0x801F78F4. That countdown word is the one
 *           the overlay step handler at 0x801F7CA8 decrements and tests against
 *           zero, so this handler arms 512 of those steps.
 *           It takes no arguments, consumes no result, and its 0x38-byte frame
 *           keeps $s0, $s1, $s2, $s3 and $ra across the one call.
 *           None of the frame bytes below 0x20 is read or written, so the
 *           original translation unit carried a declaration that emitted no
 *           code; `frame_reserve` restores the missing 0x10 bytes and nothing
 *           else.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedSampleRingAt800E5C00ClearWorkTableByte25ArmCountdown512AndAdvanceWorkByte1ScenarioScena0400_801F77EC(
    void) {
  u8* base;
  u8* record;
  u8* work;
  u8 i;
  u8 frame_reserve[0x10]; /* size-only frame reservation, see the note above */

  base = PSX_PTR(u8, 0x800E5C00u);
  *(s32*)(base + 0) = D_80147A8C;
  *(s32*)PSX_PTR(s32, 0x800E5C04u) = D_80147A90;
  *(s32*)PSX_PTR(s32, 0x800E5C08u) = D_80147A94 >> 8;
  i = 0u;
  do {
    record = base + (i << 4);
    *(s32*)(record + 0x10) = *(s32*)(base + 0) + D_801FA1A4[i];
    *(s32*)(record + 0x14) = *(s32*)(base + 4) + D_801FA1C4[i];
    *(s32*)(record + 0x18) =
        func_8015477C(*(s32*)(record + 0x10), *(s32*)(record + 0x14)) << 8;
    i += 1u;
  } while (i < 8u);
  clearWorkTableByte25At800E4800ScenarioScena0400_801F7FCC();
  work = SPAD_PTR_SLOT(u8, 0x44u);
  *(s32*)(work + 0xC) = 0x200;
  work[1]++;
}
