#include "internal.h"

/* @source 0x801DFC20
 * @behavior copies bytes 0x4b and 0x58 from the non-volatile scratchpad
 * pointer cell at 0x1f800044 into the active local-work object, clears byte 4,
 * and sets byte 2 through a separately reloaded cell.
 */
void copyScratchToLocalWork(void) {
  volatile Battle03LocalWork* work;
  u8*                         scratch_work;

  D_80146250->unk_123 = battleWork[0x4b];
  scratch_work = battleWork;
  work = D_80146250;
  work->unk_134 = *(u16*)(scratch_work + 0x58);
  scratch_work[4] = 0;
  SPAD_PTR_SLOT(u8, 0x44)[2] = 1;
}
