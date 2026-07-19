#include "internal.h"

extern void (*D_80183248[])(void);
extern CdlLOC       D_8018B490;
extern volatile u32 D_80146450;
extern volatile u32 D_80146454;
extern volatile u32 D_8014645C;
extern volatile u16 D_80146460;
extern volatile u32 D_80146464;
extern volatile u32 D_8014646C;
extern volatile u8  D_80146480;
extern volatile u8  D_80146481;
extern s8           D_80146489;
extern u8           D_80146494;
extern volatile u32 D_80146518[];
extern volatile u8  D_801464A0[];
extern volatile u32 D_80146808;

/* @behavior services the active EMI CD-ready callback, validates the sector source,
 * dispatches the current loader phase, and advances the streaming ring.
 * @source 0x80162230
 */
void func_80162230(u8 status, u8* result) {
  u8*     read_progress;
  CdlLOC* current_pos;
  s32     callback_ready;
  u32     transfer_size;
  u16     state;
  s32     slot;

  read_progress = &D_80146494;
  callback_ready = *read_progress;
  if (callback_ready != 1) {
    return;
  }

  if (CdReady(1, NULL) == callback_ready) {
    current_pos = &D_8018B490;
    CdGetSector(current_pos, 3);
    if (D_80146808 != CdPosToInt(current_pos)) {
      goto fail;
    }
  } else {
    goto fail;
  }

  state = D_80146460;
  if ((state == 0) || (state == 4) || (state == 6) || ((u16)(state - 8) < 2) ||
      (state == 10)) {
    D_80183248[D_80146460]();
  } else {
    slot = D_80146489;
    if ((D_801464A0[slot] & 0x80) == 0) {
    fail:
      D_80146480 = 1;
      *read_progress = 0;
      return;
    }

    transfer_size = D_80146454;
    if (transfer_size >= 0x801) {
      D_80146454 = transfer_size - 0x800;
      D_80146518[slot] = 0x800;
    } else {
      D_80146518[slot] = transfer_size;
      D_80146454 = 0;
    }

    slot = D_80146489;
    CdGetSector((void*)(D_80146464 + ((u32)slot << 11)), 0x200);
    if ((D_8014646C == 0) &&
        ((D_8014645C != *(u32*)(D_80146464 + ((u32)slot << 11))) ||
         (D_80146450 != D_80146808))) {
      D_80146480 = 1;
      *read_progress = 0;
      return;
    }

    D_80183248[D_80146460]();
    slot = D_80146489;
    slot += 1;
    D_80146489 = slot;
    if ((s8)slot == 0x18) {
      D_80146489 = 1;
    }
  }

  if (D_80146480 == 0) {
    if (D_80146454 == 0) {
      slot = D_80146481;
      D_80146481 = slot + 1;
      if (func_80162B08(slot) == 0) {
        func_80162CD8();
      }
    }

    D_80146808 += 1;
  }
}
