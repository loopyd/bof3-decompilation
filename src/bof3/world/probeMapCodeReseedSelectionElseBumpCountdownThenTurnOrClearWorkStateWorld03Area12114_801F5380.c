#include "bof3/bof3.h"

/* @source 0x1F800044 @kind unknown */
extern u8* area12114WorkCursor_1F800044;
/* @source 0x80143F02 @kind unknown */
extern u8 D_80143F02;
/* @source 0x80143F10 @kind unknown */
extern u16 D_80143F10;
/* @source 0x80143F14 @kind unknown */
extern u32 D_80143F14;
/* @source 0x80143F18 @kind unknown */
extern u32 D_80143F18;
/* @source 0x80143F1C @kind unknown */
extern u8 D_80143F1C;
/* @source 0x80143F1D @kind unknown */
extern u8 D_80143F1D;
/* @source 0x80143F1E @kind unknown */
extern u8 D_80143F1E;
/* @source 0x80143F22 @kind unknown */
extern u16 D_80143F22;
/* @source 0x80145AB4 @kind unknown */
extern u16 D_80145AB4;
/* @source 0x80145AB6 @kind unknown */
extern u16 D_80145AB6;
/* @source 0x80146250 @kind unknown */
extern u8* D_80146250;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;
/* @source 0x8014625A @kind unknown */
extern u16 D_8014625A;
/* @source 0x8014625C @kind unknown */
extern u16 D_8014625C;

u8 func_80166CB0(s16 arg0, s16 arg1);
void func_8019FA28(u16 selection_seed, u32 context_a, u32 context_b,
                   u8 context_kind);
void func_801A0048(s16 arg0, s16 arg1);
s32 func_801A79A4(s32 arg0, s32 arg1);
void func_801C2BEC(void);
void func_801C329C(void);
void func_801C3598(void);
void resetMode3AndWorkStateWorld03Area12114_801F564C(void);
void turnWorkFacingOneStepTowardSavedThenDispatchStepOutcomeWorld03Area12114_801F4B4C(
    void);

/* @source 0x801F5380
 * @behavior Runs the shared input-hold release at 0x801C3598 and the shared
 * update at 0x801C2BEC, then, while the front-end return flag 0x80143F02 has
 * bit 0 set, probes the map byte at the cursor halfwords 0x36 and 0x3A of the
 * scratchpad pointer cell 0x1F800044: code 0xAF re-enters the authored
 * selection with the stored context words 0x80143F10/0x80143F14/0x80143F18
 * and kind 0x80143F1C, clears the pending overlay byte 0x80143F1E and stores
 * request kind 0xC in 0x80143F1D before returning; code 0xC0 instead hands the
 * same cursor pair to 0x801A0048. It then advances the shared countdown
 * 0x80143F22 by one, by two when work byte 0x08 is 2 or 6, runs the mode 3
 * reset at 0x801F564C and asks 0x801A79A4 about the cursor position words 0x34
 * and 0x38. A nonzero answer, or the shared update 0x801C329C followed by any
 * of gate word 0x80146258 bit 0x100, 0x8014625A bit 0x40 or the overloaded
 * 0x80145AB4/0x80145AB6 pair masked with 0x8014625C and 0xF000 coming out
 * empty, clears byte 0x12B of the record published at 0x80146250 and work byte
 * 0x02; otherwise it runs the facing step at 0x801F4B4C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void probeMapCodeReseedSelectionElseBumpCountdownThenTurnOrClearWorkStateWorld03Area12114_801F5380(
    void) {
  u16 countdown;

  func_801C3598();
  func_801C2BEC();
  if (D_80143F02 & 1u) {
    if ((u8)func_80166CB0(*(s16*)(area12114WorkCursor_1F800044 + 0x36),
                          *(s16*)(area12114WorkCursor_1F800044 + 0x3A)) == 0xAF) {
      func_8019FA28(D_80143F10, D_80143F14, D_80143F18, D_80143F1C);
      D_80143F1E = 0;
      D_80143F1D = 0xC;
      return;
    }
    if ((u8)func_80166CB0(*(s16*)(area12114WorkCursor_1F800044 + 0x36),
                          *(s16*)(area12114WorkCursor_1F800044 + 0x3A)) == 0xC0) {
      func_801A0048(*(s16*)(area12114WorkCursor_1F800044 + 0x36),
                    *(s16*)(area12114WorkCursor_1F800044 + 0x3A));
    }
  }

  countdown = D_80143F22;
  D_80143F22 = countdown + 1;
  if (area12114WorkCursor_1F800044[8] == 2 ||
      area12114WorkCursor_1F800044[8] == 6) {
    D_80143F22 = countdown + 2;
  }

  resetMode3AndWorkStateWorld03Area12114_801F564C();
  if (func_801A79A4(*(s32*)(area12114WorkCursor_1F800044 + 0x34),
                    *(s32*)(area12114WorkCursor_1F800044 + 0x38))) {
    area12114WorkCursor_1F800044[2] = 0;
    return;
  }

  func_801C329C();
  if ((D_80146258 & 0x100u) == 0 && (D_8014625A & 0x40u) == 0 &&
      (((D_80145AB4 | 0xF000u) | D_80145AB6) & D_8014625C) != 0) {
    turnWorkFacingOneStepTowardSavedThenDispatchStepOutcomeWorld03Area12114_801F4B4C();
  } else {
    D_80146250[0x12B] = 0;
    area12114WorkCursor_1F800044[2] = 0;
  }
}
