#include "internal.h"

extern u8   D_800E4800;
extern vu32 D_80146454;
extern vu32 D_80146464;
extern vu32 D_80146468;
extern vu8  D_80146481;
extern vu8  D_80146482;
extern vu8  D_80146486;
extern vu8  D_80146488;
extern vu8  D_80146489;
extern vu8  D_8014648B;
extern vu8  D_80146494;
extern u8   D_80146498[];
extern vu8  D_801464A0[];
extern vu32 D_80146678;
extern u8   D_80146840;
extern vu32 D_80146808;
extern vu32 D_80146858;
extern vu32 D_8014685C;
/* @behavior initializes EMI streaming state, refreshes the active LBA, and installs
 * CD callbacks.
 * @source 0x80161fdc FUN_80161fdc
 */
void func_80161fdc(u32 slot_id) {
  u32 cd_base;
  u32 active_lba;
  u32 current_lba;
  u32 max_lba;
  u32 slot_index;
  u8  slot;

  cd_base = 0x800E4800;
  slot_index = 0;
  D_80146468 = slot_id;
  D_80146481 = 0;
  D_80146482 = 0;
  D_80146486 = 0;
  D_80146488 = 1;
  D_80146489 = 0;
  D_80146464 = cd_base;

  do {
    D_801464A0[slot_index] = 0xff;
    slot_index += 1;
  } while (slot_index < 0x18);

  if (D_80146840 == 2) {
    if (CdSync(1, D_80146498) != 0) {
      if ((D_80146498[0] & 0xe0) != 0) {
        D_8014648B = 1;
      }
    }
  }

  max_lba = D_8014685C;
  current_lba = D_80146858;
  D_80146840 = 1;
  if (max_lba < current_lba) {
    D_8014685C = current_lba;
  }

  active_lba = D_80146468;
  D_80146858 = 0;
  current_lba = func_80162160(active_lba);
  slot = D_80146481;
  D_80146678 = current_lba;
  D_80146808 = current_lba;
  D_80146481 = slot + 1;
  func_80162b08(slot);

  D_80146454 = 0x800;
  D_80146494 = 0;
  CdReadyCallback(emi_cd_ready_callback);
  CdSyncCallback((CdlCB)emi_cd_sync_callback);
  func_80162178();
}
