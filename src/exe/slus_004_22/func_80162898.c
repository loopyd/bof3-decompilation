#include "internal.h"

extern u8 DAT_80140000[];

#define EMI_LOADER_STEP (*(volatile u32*)(DAT_80140000 + 0x646c))

extern vu32           DAT_8014646c;
extern vu8            DAT_80146483;
extern signed char     DAT_80146489;
extern vu8            DAT_801464a0[];
extern vu32           DAT_80146478;
extern vu8            DAT_80146481;
extern vu8            DAT_80146485;
extern vu8            DAT_80146486;
extern vu8            DAT_80146480;
extern vu8            DAT_80146494;
extern vu32           DAT_80146454;
extern EmiLoaderEntry DAT_8014677c[];

/* @behavior starts the current EMI entry transfer and records either its
 * active-slot state or a terminal loader error before advancing the step.
 * @source 0x80162898 FUN_80162898
 */
void func_80162898(void) {
  u32 read_cursor;
  u8  entry_index;
  u8  source_index;

  if (EMI_LOADER_STEP == 0) {
    func_801690b8(0);
    func_80173818(DAT_8014677c[DAT_80146483].resource_id);
    entry_index = DAT_80146483;
    if ((s16)func_80173c50(DAT_8014677c[entry_index].source,
                          DAT_8014677c[entry_index].resource_id,
                          DAT_8014677c[entry_index].unknown_00) == -1) {
      DAT_80146480 = 2;
      DAT_80146494 = 0;
      return;
    }

    *(volatile s32*)((u8*)&EMI_LOADER_STEP + 0x4c + (DAT_80146489 * 4)) = -1;
    DAT_80146486 = 1;
    read_cursor = DAT_80146454;
    source_index = DAT_80146481;
    DAT_80146478 = read_cursor + 0x800;
    DAT_80146485 = source_index;
  }

  DAT_801464a0[DAT_80146489] = 4;
  DAT_8014646c += 1;
}
