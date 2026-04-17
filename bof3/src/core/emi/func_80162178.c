#include "internal.h"

/* clang-format off */
#include <libcd.h>
/* clang-format on */

#define BOF3_EMI_CALLBACK_BUSY   (*(volatile u8*)0x80146480u)
#define BOF3_EMI_LOADER_PHASE    (*(volatile u8*)0x8014648au)
#define BOF3_EMI_ASYNC_CD_RESUME (*(volatile s8*)0x8014648bu)
#define BOF3_EMI_RETRY_COUNTER   (*(volatile u16*)0x80146490u)
#define BOF3_EMI_CALLBACK_STATE  (*(volatile u16*)0x80146492u)
#define BOF3_EMI_READ_PROGRESS   (*(volatile u8*)0x80146494u)
#define BOF3_EMI_CURRENT_LOC     ((CdlLOC*)0x80146778u)

extern vu32 DAT_80146808;

/* does: resets EMI transfer counters, converts the current LBA to CdlLOC, and
 * arms the next loader phase.
 * @source: 0x80162178 FUN_80162178
 */
void func_80162178(void) {
  volatile u8* read_progress;
  u32          lba;
  u8           loader_phase;

  read_progress = (volatile u8*)0x80146494u;

  *read_progress = 0;
  lba = DAT_80146808;
  BOF3_EMI_RETRY_COUNTER = 0;
  CdIntToPos(lba, (CdlLOC*)(read_progress + 0x2e4));
  read_progress = (volatile u8*)0x80146480u;
  BOF3_EMI_CALLBACK_STATE = 3;
  *read_progress = 0;

  loader_phase = 1;
  if (BOF3_EMI_ASYNC_CD_RESUME == 1) {
    loader_phase = 6;
  }

  BOF3_EMI_LOADER_PHASE = loader_phase;
}
