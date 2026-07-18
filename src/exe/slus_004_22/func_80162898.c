#include "internal.h"

extern u8 D_80140000[];

#define EMI_LOADER_STEP (*(volatile u32*)(D_80140000 + 0x646c))

extern vu32           D_8014646C;
extern vu8            D_80146483;
extern signed char    D_80146489;
extern vu8            D_801464A0[];
extern vu32           D_80146478;
extern vu8            D_80146481;
extern vu8            D_80146485;
extern vu8            D_80146486;
extern vu8            D_80146480;
extern vu8            D_80146494;
extern vu32           D_80146454;
extern EmiLoaderEntry D_8014677C[];

/* @behavior starts the current EMI entry transfer and records either its
 * active-slot state or a terminal loader error before advancing the step.
 * @source 0x80162898
 */
void func_80162898(void) {
  u32 read_cursor;
  u8  entry_index;
  u8  source_index;

  if (EMI_LOADER_STEP == 0) {
    SpuSetTransferMode(0);
    SsVabClose(D_8014677C[D_80146483].resource_id);
    entry_index = D_80146483;
    if ((s16)SsVabOpenHeadSticky(D_8014677C[entry_index].source,
                                 D_8014677C[entry_index].resource_id,
                                 D_8014677C[entry_index].unknown_00) == -1) {
      D_80146480 = 2;
      D_80146494 = 0;
      return;
    }

    *(volatile s32*)((u8*)&EMI_LOADER_STEP + 0x4c + (D_80146489 * 4)) = -1;
    D_80146486 = 1;
    read_cursor = D_80146454;
    source_index = D_80146481;
    D_80146478 = read_cursor + 0x800;
    D_80146485 = source_index;
  }

  D_801464A0[D_80146489] = 4;
  D_8014646C += 1;
}
