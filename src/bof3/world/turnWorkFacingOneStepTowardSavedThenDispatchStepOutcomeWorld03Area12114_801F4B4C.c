#include "bof3/bof3.h"

/* Scratchpad pointer cell 0x1F800044 holding the active work record. The cell is
 * a variable rather than a constant address so that each use reloads it, as the
 * original does before the work byte 0x0A gate, before the saved-facing compare,
 * before each of the two state stores and before every step store on the two
 * dispatch arms.
 * @source 0x1F800044 @kind unknown
 */
extern u8* area12114WorkCursor_1F800044;

/* Shared mode byte whose zero lets this step run.
 * @source 0x80143BB0 @kind unknown
 */
extern u8 D_80143BB0;

/* Overlapping pad halfwords of the shared main executable. The body only tests
 * whether the two halfwords share a set bit; no bit of either is named here.
 * @source 0x80145AA4 @kind unknown
 */
extern u16 D_80145AA4;
/* @source 0x80145AB6 @kind unknown */
extern u16 D_80145AB6;

/* Record pointer cell of the shared main executable. Byte 0x11C is the record
 * mode, 0x12A and 0x12B are the two flags cleared or raised on the dispatch
 * arms, and byte 0x13C indexes the slot flag table.
 * @source 0x80146250 @kind unknown
 */
extern u8* D_80146250;

/* Shared status halfwords tested by the entry gates: bit 0x100 of the first and
 * bit 0x40 of the second stop the step, and a nonzero third one enables the
 * slot flag clear.
 * @source 0x80146258 @kind unknown
 */
extern u16 D_80146258;
/* @source 0x8014625A @kind unknown */
extern u16 D_8014625A;
/* @source 0x8014625C @kind unknown */
extern u16 D_8014625C;

/* Slot flag view of the shared record table at 0x80144974: each record is 0xA4
 * bytes and the only field this function reads is the halfword at offset 0,
 * tested for bit 0x20. The index is the byte at 0x13C of the record published
 * at 0x80146250.
 */
typedef struct Area12114SlotFlagView {
  u16 flags_00;
  u8 unk_02[0xA2];
} Area12114SlotFlagView;

/* @source 0x80144974 @kind table */
extern Area12114SlotFlagView D_80144974[];

s32 faceWorkTowardResolvedSlotThenDecrementByte9AndSetState2To1World03Area12114_801F5268(
    void);
s32 func_801B6674(void);
s32 setMode1AndClearWorkByte2IfMaskOverlapUnlessGate40ThenQueueCue105World03Area12114_801F5578(
    void);
s32 func_801B6258(void);
s32 setMode4AndClearWorkByte2IfMaskOverlapWorld03Area12114_801F55EC(void);
s32 updateWorkByte8FromMaskWorld03Area12114_801F5684(void);
void resetMode3AndWorkStateWorld03Area12114_801F564C(void);
s32 func_801B0E44(void);
s8 func_801BDF08(void);
void setResolvedSlotAnimBit0WhenFacingMatchesElseFaceAndDecrementByte9World03Area12114_801F50F4(
    void);
void setModeFromMaskOverlapAndWorkByteBWorld03Area12114_801F5734(void);
void func_801C24D0(void);
void func_801C4C48(void);
void func_801B0DFC(void);

/* @source 0x801F4B4C
 * @behavior Byte-2 state handler 0 of the two-entry handler table 0x801F689C,
 * the table the overlay step dispatch at 0x801F4B00 indexes with work byte 0x02.
 * It returns early when the facing step of 0x801F5268 reports a settled slot,
 * when bit 0x100 of halfword 0x80146258 or bit 0x40 of halfword 0x8014625A is
 * set, when shared byte 0x80143BB0 is nonzero, or when work byte 0x0A is
 * nonzero; while halfword 0x8014625C is nonzero it first clears byte 0x12A of
 * the record at 0x80146250 unless bit 0x20 of the 0xA4-byte slot flag record
 * that record byte 0x13C selects is set, and it returns early again when any of
 * the four shared guards 0x801B6674, 0x801F5578, 0x801B6258 and 0x801F55EC
 * answers nonzero. It latches work facing byte 0x08, runs the mask update at
 * 0x801F5684 and, when the pad halfwords 0x80145AA4 and 0x80145AB6 overlap or
 * work byte 0x0B reads 0x40, restores the latched facing and the record flags
 * through 0x801F564C. Otherwise an overlap restores the facing, and a facing
 * that differs from the latched byte is stepped one place by the shorter arc of
 * the eight-way facing ring, only when the two lifted values stand exactly two
 * apart, before the ring mask 7 is applied. The result of 0x801B0E44 then
 * selects the arm: 0 settles through 0x801F50F4 when 0x801BDF08 reports a
 * nonzero byte and otherwise runs 0x801F5734 before either re-publishing the
 * latched facing and leaving record byte 0x11C cleared, or advancing through
 * 0x801C24D0, 0x801C4C48, the countdown byte 0x09 and 0x801B0DFC; 1 and 0xFF
 * restore the facing, clear work bytes 0x09 and 0x02 and write 3 to record byte
 * 0x11C. Takes no arguments and returns nothing.
 * @status partial
 * @match 88.39
 * @residual The entry gates, the mask update, the facing compare, the whole
 * eight-way turn of the first stage and the complete 0x801B0E44 dispatch match
 * the original instruction for instruction, including the registers of the
 * latched facing, both sign-extended operands and the first distance. The
 * second distance is the whole residual: the original keeps it in v0, the same
 * register its first distance used, while this candidate keeps the two
 * distances in separate variables and the second one is allocated v1. That
 * makes the ring constant the low register and lets it be hoisted into both
 * branch delay slots, so the candidate is one instruction shorter and every
 * later branch displacement sits four bytes early. Writing both distances into
 * one variable did not hold, because it collapses the first stage back to the
 * v1 copy and the duplicated three-way compare; naming the second distance
 * before the first only swaps which of the two takes v1.
 */
void turnWorkFacingOneStepTowardSavedThenDispatchStepOutcomeWorld03Area12114_801F4B4C(
    void) {
  s32 saved;
  s32 now;
  s32 want;
  s32 cur;
  s32 dist;
  s32 diff;
  s32 cur8;
  s32 want8;
  s32 step;

  if ((faceWorkTowardResolvedSlotThenDecrementByte9AndSetState2To1World03Area12114_801F5268() &
       0xFF) != 0) {
    return;
  }
  if ((D_80146258 & 0x100) != 0) {
    return;
  }
  if ((D_8014625A & 0x40) != 0) {
    return;
  }
  if (D_80143BB0 != 0) {
    return;
  }
  if (area12114WorkCursor_1F800044[0xA] != 0) {
    return;
  }
  if (D_8014625C != 0) {
    if ((D_80144974[D_80146250[0x13C]].flags_00 & 0x20) == 0) {
      D_80146250[0x12A] = 0;
    }
  }
  if ((func_801B6674() & 0xFF) != 0) {
    return;
  }
  if ((setMode1AndClearWorkByte2IfMaskOverlapUnlessGate40ThenQueueCue105World03Area12114_801F5578() &
       0xFF) != 0) {
    return;
  }
  if ((func_801B6258() & 0xFF) != 0) {
    return;
  }
  if ((setMode4AndClearWorkByte2IfMaskOverlapWorld03Area12114_801F55EC() &
       0xFF) != 0) {
    return;
  }
  saved = area12114WorkCursor_1F800044[8];
  if ((((updateWorkByte8FromMaskWorld03Area12114_801F5684() & 0xFF) == 0) &&
       ((D_80145AA4 & D_80145AB6) == 0)) ||
      (area12114WorkCursor_1F800044[0xB] == 0x40)) {
    area12114WorkCursor_1F800044[8] = saved;
    D_80146250[0x12B] = 0;
    resetMode3AndWorkStateWorld03Area12114_801F564C();
    area12114WorkCursor_1F800044[9] = 0;
    area12114WorkCursor_1F800044[2] = 0;
    return;
  }
  if ((D_80145AA4 & D_80145AB6) != 0) {
    area12114WorkCursor_1F800044[8] = saved;
  }
  now = area12114WorkCursor_1F800044[8];
  if (now != saved) {
    cur = now;
    want8 = (s8)saved;
    cur8 = (s8)cur;
    dist = want8 - cur8;
    if (dist < 0) {
      dist = -dist;
    }
    want = saved;
    if (dist >= 3) {
      if (want8 < cur8) {
        want = saved + 8;
      } else {
        cur = now + 8;
      }
    }
    want8 = (s8)want;
    cur8 = (s8)cur;
    diff = want8 - cur8;
    if (diff < 0) {
      diff = -diff;
    }
    if (diff == 2) {
      if (cur8 < want8) {
        area12114WorkCursor_1F800044[8] = saved - 1;
      } else {
        area12114WorkCursor_1F800044[8] = saved + 1;
      }
    }
    area12114WorkCursor_1F800044[8] &= 7;
  }
  step = func_801B0E44() & 0xFF;
  switch (step) {
    case 0:
      if ((s8)func_801BDF08() != 0) {
        D_80146250[0x12B] = 0;
        area12114WorkCursor_1F800044[2] = 0;
        setResolvedSlotAnimBit0WhenFacingMatchesElseFaceAndDecrementByte9World03Area12114_801F50F4();
        return;
      }
      setModeFromMaskOverlapAndWorkByteBWorld03Area12114_801F5734();
      if (D_80146250[0x11C] == 4 &&
          area12114WorkCursor_1F800044[8] != saved) {
        area12114WorkCursor_1F800044[8] = saved;
        D_80146250[0x12B] = 0;
        area12114WorkCursor_1F800044[2] = 0;
        return;
      }
      func_801C24D0();
      func_801C4C48();
      area12114WorkCursor_1F800044[9] -= 1;
      func_801B0DFC();
      D_80146250[0x12B] = 1;
      area12114WorkCursor_1F800044[2] = 1;
      return;
    case 1:
    case 0xFF:
      area12114WorkCursor_1F800044[8] = saved;
      D_80146250[0x12B] = 0;
      D_80146250[0x11C] = 3;
      area12114WorkCursor_1F800044[9] = 0;
      area12114WorkCursor_1F800044[2] = 0;
      return;
  }
}
