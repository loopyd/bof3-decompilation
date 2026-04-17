#include "internal.h"

/* clang-format off */
#include <libcd.h>
/* clang-format on */

u32         func_80162160(u32 arg0);
void        func_80162178(void);
void        func_80162b08(u8 arg0);
extern u8   DAT_800e4800;
extern vu32 DAT_80146454;
extern vu32 DAT_80146464;
extern vu32 DAT_80146468;
extern vu8  DAT_80146481;
extern vu8  DAT_80146482;
extern vu8  DAT_80146486;
extern vu8  DAT_80146488;
extern vu8  DAT_80146489;
extern vu8  DAT_8014648b;
extern vu8  DAT_80146494;
extern u8   DAT_80146498[];
extern vu8  DAT_801464a0[];
extern vu32 DAT_80146678;
extern u8   DAT_80146840;
extern vu32 DAT_80146808;
extern vu32 DAT_80146858;
extern vu32 DAT_8014685c;
extern void LAB_801621e8(void);
extern void LAB_80162230(void);
/* does: initializes EMI streaming state, refreshes the active LBA, and installs
 * CD callbacks.
 * @source: 0x80161fdc FUN_80161fdc
 */
void emi_stream_init_slot(u32 slot_id) {
  u32 cd_base;
  u32 active_lba;
  u32 current_lba;
  u32 max_lba;
  u32 slot_index;
  u8  slot;

  cd_base = 0x800E4800;
  slot_index = 0;
  DAT_80146468 = slot_id;
  DAT_80146481 = 0;
  DAT_80146482 = 0;
  DAT_80146486 = 0;
  DAT_80146488 = 1;
  DAT_80146489 = 0;
  DAT_80146464 = cd_base;

  do {
    DAT_801464a0[slot_index] = 0xff;
    slot_index += 1;
  } while (slot_index < 0x18);

  if (DAT_80146840 == 2) {
    if (CdSync(1, DAT_80146498) != 0) {
      if ((DAT_80146498[0] & 0xe0) != 0) {
        DAT_8014648b = 1;
      }
    }
  }

  max_lba = DAT_8014685c;
  current_lba = DAT_80146858;
  DAT_80146840 = 1;
  if (max_lba < current_lba) {
    DAT_8014685c = current_lba;
  }

  active_lba = DAT_80146468;
  DAT_80146858 = 0;
  current_lba = func_80162160(active_lba);
  slot = DAT_80146481;
  DAT_80146678 = current_lba;
  DAT_80146808 = current_lba;
  DAT_80146481 = slot + 1;
  func_80162b08(slot);

  DAT_80146454 = 0x800;
  DAT_80146494 = 0;
  CdReadyCallback((CdlCB)&LAB_80162230);
  CdSyncCallback((CdlCB)&LAB_801621e8);
  func_80162178();
}
