#include "internal.h"

extern void (*D_80183248[])(void);
extern CdlLOC       cdCurrentPositionState; /* @kind: bss */
extern volatile u32 emiExpectedSectorState;   /* @kind: bss */
extern volatile u32 D_80146454;
extern volatile u32 D_8014645C;
extern volatile u16 D_80146460;
extern volatile u32 D_80146464;
extern volatile u32 D_8014646C;
extern volatile u8  D_80146480;
extern volatile u8  D_80146481;
extern s8           D_80146489;
extern u8           D_80146494;
extern volatile u8  D_801464A0[];
extern volatile u32 D_80146808;

/* @behavior services the active EMI CD-ready callback, validates the sector source,
 * dispatches the current loader phase, and advances the streaming ring.
 * @source 0x80162230
 */
void emi_cd_ready_callback(u8 status, u8* result) {
  u8*           read_progress;
  CdlLOC*       current_pos;
  s32           callback_ready;
  u32           transfer_size;
  u16           state;
  u8            next;

  read_progress = &D_80146494;
  callback_ready = *read_progress;
  if (callback_ready != 1) {
    return;
  }

  if (CdReady(1, NULL) == callback_ready) {
    current_pos = &cdCurrentPositionState;
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
    /*
     * MATCHING_AID: the original derives the slot_sizes base as
     * read_progress+0x84 in $v1 (filling the sltiu-branch delay slot),
     * which rotates the ring slot to $a1 and lets $v1 be reused for the
     * 0x800 constant; clean C allocates the base to $a1 and the slot to
     * $v1. The pointee stays non-volatile: a volatile store can never
     * move into the jump delay slot (reorg resource_conflicts_p), but the
     * original stores 0x800 there. Clean-C reorderings, hoists,
     * local/global variants, a bounded permuter run, profile and
     * historical-compiler probes were exhausted; without the pin the live
     * diff is exactly this $v1/$a1 swap. Remove when the allocator web is
     * understood.
     */
    REGISTER_PIN(u32*, slot_sizes, "v1");
    s32 slot = D_80146489;
    if ((D_801464A0[slot] & 0x80) == 0) {
    fail:
      D_80146480 = 1;
      *read_progress = 0;
      return;
    }

    transfer_size = D_80146454;
    slot_sizes = (u32*)(read_progress + 0x84);
    if (transfer_size >= 0x801) {
      D_80146454 = transfer_size - 0x800;
      slot_sizes[slot] = 0x800;
    } else {
      slot_sizes[slot] = transfer_size;
      D_80146454 = 0;
    }

    /* Reusing the dead transfer_size for the reload keeps the CdGetSector
     * argument setup in $a0 without a spare temporary. */
    transfer_size = D_80146489;
    CdGetSector((void*)(D_80146464 + (transfer_size << 11)), 0x200);
    if (D_8014646C == 0) {
      u32* sector_head = (u32*)(((u32)D_80146489 << 11) + D_80146464);
      if ((D_8014645C != *sector_head) || (emiExpectedSectorState != D_80146808)) {
        D_80146480 = 1;
        D_80146494 = 0;
        return;
      }
    }

    D_80183248[D_80146460]();
    {
      s8* slot_p = &D_80146489;
      *slot_p += 1;
      if (*slot_p == 0x18) {
        *slot_p = 1;
      }
    }
  }

  if (D_80146480 == 0) {
    if (D_80146454 == 0) {
      next = D_80146481;
      D_80146481 = next + 1;
      if (emi_stage_transfer_slot(next) == 0) {
        emi_loader_select_mode6();
      }
    }

    {
      volatile u32* expected_pos = &D_80146808;
      *expected_pos += 1;
    }
  }
}
