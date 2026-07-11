#include "internal.h"

#define EMI_CURRENT_POS ((CdlLOC*)0x8018b490u)

s32  func_80162b08(u8 slot);
void func_80162cd8(void);

extern void (*DAT_80183248[])(void);
extern vu32 DAT_80146450;
extern vu32 DAT_80146454;
extern vu32 DAT_8014645c;
extern vu16 DAT_80146460;
extern vu32 DAT_80146464;
extern vu32 DAT_8014646c;
extern vu8  DAT_80146480;
extern vu8  DAT_80146481;
extern vu8  DAT_80146489;
extern vu8  DAT_80146494;
extern vu32 DAT_80146518[];
extern vu8  DAT_801464a0[];
extern vu32 DAT_80146808;

/* @behavior services the active EMI CD-ready callback, validates the sector source,
 * dispatches the current loader phase, and advances the streaming ring.
 * @source 0x80162230 FUN_80162230
 */
void func_80162230(u8 status, u8* result) {
  u16 state;
  u8  slot;

  if (DAT_80146494 != 1) {
    return;
  }

  if (CdReady(1, NULL) == 1) {
    CdGetSector(EMI_CURRENT_POS, 3);
    if (DAT_80146808 != CdPosToInt(EMI_CURRENT_POS)) {
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
      goto fail;
    }

    if (DAT_80146454 < 0x801) {
      DAT_80146518[slot] = DAT_80146454;
      DAT_80146454 = 0;
    } else {
      DAT_80146454 -= 0x800;
      DAT_80146518[slot] = 0x800;
    }

    CdGetSector((void*)(DAT_80146464 + ((u32)slot << 11)), 0x200);
    if ((DAT_8014646c == 0) &&
        ((DAT_8014645c != *(u32*)(DAT_80146464 + ((u32)slot << 11))) ||
         (DAT_80146450 != DAT_80146808))) {
      goto fail;
    }

    DAT_80183248[DAT_80146460]();
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

  return;

fail:
  DAT_80146480 = 1;
  DAT_80146494 = 0;
}
