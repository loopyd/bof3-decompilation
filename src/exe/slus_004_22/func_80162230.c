#include "internal.h"

extern void (*DAT_80183248[])(void);
extern CdlLOC DAT_8018b490;
extern vu32   DAT_80146450;
extern vu32   DAT_80146454;
extern vu32   DAT_8014645c;
extern vu16   DAT_80146460;
extern vu32   DAT_80146464;
extern vu32   DAT_8014646c;
extern vu8    DAT_80146480;
extern vu8    DAT_80146481;
extern s8     DAT_80146489;
extern u8     DAT_80146494;
extern vu32   DAT_80146518[];
extern vu8    DAT_801464a0[];
extern vu32   DAT_80146808;

/* @behavior services the active EMI CD-ready callback, validates the sector source,
 * dispatches the current loader phase, and advances the streaming ring.
 * @source 0x80162230 FUN_80162230
 */
void func_80162230(u8 status, u8* result) {
  u8*     read_progress;
  CdlLOC* current_pos;
  s32     callback_ready;
  u32     transfer_size;
  u16     state;
  s32     slot;

  read_progress = &DAT_80146494;
  callback_ready = *read_progress;
  if (callback_ready != 1) {
    return;
  }

  if (CdReady(1, NULL) == callback_ready) {
    current_pos = &DAT_8018b490;
    CdGetSector(current_pos, 3);
    if (DAT_80146808 != CdPosToInt(current_pos)) {
      goto fail;
    }
  } else {
    goto fail;
  }

  state = DAT_80146460;
  if ((state == 0) || (state == 4) || (state == 6) || ((u16)(state - 8) < 2) ||
      (state == 10)) {
    DAT_80183248[DAT_80146460]();
  } else {
    slot = DAT_80146489;
    if ((DAT_801464a0[slot] & 0x80) == 0) {
    fail:
      DAT_80146480 = 1;
      *read_progress = 0;
      return;
    }

    transfer_size = DAT_80146454;
    if (transfer_size >= 0x801) {
      DAT_80146454 = transfer_size - 0x800;
      ((vu32*)((u8*)read_progress + 132))[slot] = 0x800;
    } else {
      ((vu32*)((u8*)read_progress + 132))[slot] = transfer_size;
      DAT_80146454 = 0;
    }

    slot = DAT_80146489;
    CdGetSector((void*)(DAT_80146464 + ((u32)slot << 11)), 0x200);
    if ((DAT_8014646c == 0) &&
        ((DAT_8014645c != *(u32*)(DAT_80146464 + ((u32)slot << 11))) ||
         (DAT_80146450 != DAT_80146808))) {
      DAT_80146480 = 1;
      *read_progress = 0;
      return;
    }

    DAT_80183248[DAT_80146460]();
    slot = DAT_80146489;
    slot += 1;
    DAT_80146489 = slot;
    if ((s8)slot == 0x18) {
      DAT_80146489 = 1;
    }
  }

  if (DAT_80146480 == 0) {
    if (DAT_80146454 == 0) {
      slot = DAT_80146481;
      DAT_80146481 = slot + 1;
      if (func_80162b08(slot) == 0) {
        func_80162cd8();
      }
    }

    DAT_80146808 += 1;
  }
}
