#include "bof3/ui/shop00_internal.h"

/* @source 0x801D62DC
 * @behavior Shop UI phase step at entry 1 of the phase step table D_801E5360,
 *           which the dispatcher func_801D6184 selects with the UI phase byte
 *           D_80148651; the entry-0 step
 *           publishShopRowFlagsAndAdvanceUiPhaseUnlessFlag77Set advances that
 *           byte into this step. It runs only while the shared gate halfword
 *           D_80143C40 reads zero, and every run starts by appending the
 *           fullscreen dim tile through appendFullscreenDimTile. It then divides
 *           on the world/selection id D_80143F00: for the ids 0xBB and 0xC1, and
 *           for 0xBF while the shared scenario-state byte D_80146870 reads 0xE
 *           and flag 5 of the work-context word D_8014686C passes the shared flag
 *           tester func_8015B5D4, it walks the four overlay halfwords D_801E6100
 *           with an unsigned byte counter and calls the main-exe per-entry helper
 *           func_8016BB6C with the constant 0 and the row index of every nonzero
 *           entry. Every other id takes the rebuild arm: it stores 0xFF into the
 *           main-RAM record index byte D_80145029 (the same none-selected value
 *           the battle overlays store there) and picks the group of the
 *           sixteen-byte table D_801E537C - 1 after clearing save flag 0x77 of
 *           the flag bank D_80144F28 through func_8015B5A8 when the id is 0x85,
 *           2 when the id is 0xBF, and 0 otherwise. It then clears the low three
 *           bits of the shared world-flag halfword D_8014625A, stores the mode
 *           slot count func_801BDB7C(0) into the record-count byte D_80146254
 *           and switches the frontend mode through func_801C1400 with the
 *           constant 0. While that count is nonzero it walks the 0x140-byte work
 *           records at D_80145E90 in index order: it publishes the record
 *           pointer into the scratchpad work cell 0x1F800044, hands the record's
 *           byte at +0x79 (carried by its own base D_80145F09) and the record
 *           index to the shared icon-record helper func_801C1254, requests the
 *           resource named by the record's byte at +0x08 through func_8014D6B8,
 *           copies the three words of the group/slot record
 *           D_801E537C[group * 3 + slot] into the work record's +0x34, +0x38 and
 *           +0x3C words and clears its bytes at +0x01 through +0x04. It finishes
 *           by copying the main-RAM player position words D_80145EC4 and
 *           D_80145EC8 into the shared position pair D_80149308 and D_8014930C,
 *           resetting the shared overlay through func_80153DE8, and submitting
 *           the first word of the record that the pointer table D_8017F974
 *           selects with D_80143F00 through func_801A3E50. Both arms end by
 *           advancing the UI phase byte D_80148651 by one, which hands the next
 *           frame to entry 2 func_801D65A0, while a set gate halfword leaves that
 *           byte untouched so the step repeats. The phase byte is read and
 *           written once per run and the three table words are the only stores
 *           into the work record besides the four cleared bytes. Takes no
 *           arguments and returns nothing; its 0x20-byte frame keeps only the
 *           return address and the three saved registers the two walks need.
 * @status partial
 * @match 89.33
 * @residual The body compiles to 712 bytes against the original 708. Three
 *           placement residuals remain, all of them code-motion decisions of
 *           the owning compiler rather than source facts. First, the rebuild
 *           arm: the original stores 0xFF into D_80145029 and only then reloads
 *           D_80143F00, entering that block once at its head, while this build
 *           hoists the reload above the store, folds the store address into
 *           $at instead of the register the original reuses to derive
 *           D_80144F28, and threads the 0xBF branch past the reload, which
 *           shifts every later label by three instructions. Second, the
 *           four-halfword walk: this build masks the row once and shares that
 *           value between the D_801E6100 index and the func_8016BB6C argument,
 *           where the original masks both uses separately and so fills the
 *           branch and call delay slots differently. Third, the work-record
 *           walk: the third table word is stored through the work cell reloaded
 *           for the next byte store, one load-delay nop later than the original,
 *           which stores it through the base it already holds. The table
 *           strides, every literal, the two walks, the arm split and the tail
 *           publishes do match.
 */
void setUpShopRowsOrSeedWorkRecordTriplesThenResetOverlayAndAdvanceUiPhase(void) {
  u8* phaseCell;
  u8 group;
  u8 row;
  u8 i;
  u32 triple;

  if (D_80143C40 != 0) {
    return;
  }
  appendFullscreenDimTile();
  if (D_80143F00 == 0xBB || D_80143F00 == 0xC1 ||
      (D_80143F00 == 0xBF && D_80146870 == 0xE &&
       func_8015B5D4(D_8014686C, 5) != 0)) {
    for (row = 0; row < 4; row++) {
      if (D_801E6100[row] != 0) {
        func_8016BB6C(0, row);
      }
    }
  } else {
    group = 0;
    D_80145029 = 0xFF;
    if (D_80143F00 == 0x85) {
      func_8015B5A8(&D_80144F28, 0x77);
      group = 1;
    }
    if (D_80143F00 == 0xBF) {
      group = 2;
    }
    D_8014625A &= 0xFFF8;
    D_80146254 = func_801BDB7C(0);
    func_801C1400(0);
    for (i = 0; i < D_80146254; i++) {
      D_1F800044 = &D_80145E90[i * 0x140];
      func_801C1254(D_80145F09[i * 0x140], i);
      func_8014D6B8(D_1F800044[8]);
      *(u32*)(D_1F800044 + 0x34) = D_801E537C[group][i][0];
      *(u32*)(D_1F800044 + 0x38) = D_801E537C[group][i][1];
      /* The third word keeps its own name because the original loads it before
       * the first byte clear and stores it after the work cell is reloaded. */
      triple = D_801E537C[group][i][2];
      D_1F800044[1] = 0;
      *(u32*)(D_1F800044 + 0x3C) = triple;
      D_1F800044[2] = 0;
      D_1F800044[3] = 0;
      D_1F800044[4] = 0;
    }
    D_80149308 = D_80145EC4;
    D_8014930C = D_80145EC8;
    func_80153DE8();
    func_801A3E50(*(u32*)D_8017F974[D_80143F00]);
  }
  /* The original materializes the phase-byte address once and reuses that base
   * for the increment load and store (lui plus addiu, then lbu and sb through
   * one register), which a direct D_80148651 += 1 does not: that spelling folds
   * the low half into the load and stores through $at, as the exact phase step
   * func_801D65A0 shows. The exact entry-0 phase step
   * publishShopRowFlagsAndAdvanceUiPhaseUnlessFlag77Set takes this same pointer
   * form for this same byte. */
  phaseCell = (u8*)&D_80148651;
  *phaseCell += 1;
}
