#include "bof3/core/slus_internal.h"

extern u8           D_800E4800;
extern volatile u32 D_80146454;
extern volatile u32 D_80146464;
extern volatile u32 D_80146468;
extern volatile u8  D_80146481;
extern volatile u8  D_80146482;
extern volatile u8  D_80146486;
extern volatile u8  D_80146488;
extern volatile u8  D_80146489;
extern volatile u8  D_8014648B;
extern volatile u8  D_80146494;
extern u8           D_80146498[];
extern volatile u8  D_801464A0[];
extern volatile u32 D_80146678;
extern u8           D_80146840;
extern volatile u32 D_80146808;
extern volatile u32 D_80146858;
extern volatile u32 D_8014685C;
/* @behavior initializes EMI streaming state, refreshes the active LBA, and installs
 * CD callbacks.
 * @source 0x80161FDC
 * @status exact
 * @match 100.00
 * @residual none
 */
void initStreamSlot(u32 slot_id) {
  u32 cdBase;
  u32 current_lba;
  u32 active_lba;
  u32 max_lba;
  u32 slot_index;
  u8  slot;

  cdBase = 0x800E4800;
  slot_index = 0;
  D_80146468 = slot_id;
  D_80146481 = 0;
  D_80146482 = 0;
  D_80146486 = 0;
  D_80146488 = 1;
  D_80146489 = 0;
  D_80146464 = cdBase;

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

  current_lba = D_80146468;
  active_lba = current_lba;
  D_80146858 = 0;
  active_lba = emiLoaderSlotLba(active_lba);
  slot = D_80146481;
  D_80146678 = active_lba;
  D_80146808 = active_lba;
  D_80146481 = slot + 1;
  stageEmiTransferSlot(slot);

  D_80146454 = 0x800;
  D_80146494 = 0;
  CdReadyCallback(emiCdReadyCallback);
  CdSyncCallback((CdlCB)emiCdSyncCallback);
  beginEmiLoaderTransfer();
}
