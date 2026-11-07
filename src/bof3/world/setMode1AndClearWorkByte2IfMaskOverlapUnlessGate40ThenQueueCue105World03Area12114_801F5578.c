#include "bof3/bof3.h"

/* @source 0x80145AA8 @kind unknown */
extern u16 D_80145AA8;
/* @source 0x80145AB8 @kind unknown */
extern u16 D_80145AB8;
/* @source 0x80146258 @kind unknown */
extern u16 D_80146258;
/* @source 0x80143BB0 @kind unknown */
extern u8 D_80143BB0;

/* Sound-cue dispatcher of the main executable, outside this overlay's
 * payload. */
void func_8015DF18(u16 cue);

/* @source 0x801F5578
 * @behavior Returns one when the halfwords at 0x80145AA8 and 0x80145AB8 share
 * a set bit and halfword 0x80146258 has bit 0x40 clear. On that path it also
 * dispatches sound cue 0x105, writes 1 to the mode byte 0x80143BB0 and clears
 * byte 2 of the work record at scratchpad pointer slot 0x44. Otherwise it
 * returns zero. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 setMode1AndClearWorkByte2IfMaskOverlapUnlessGate40ThenQueueCue105World03Area12114_801F5578(void) {
  u8* record;

  if ((D_80145AA8 & D_80145AB8) != 0) {
    if ((D_80146258 & 0x40u) == 0) {
      func_8015DF18(0x105);
      record = SPAD_PTR_SLOT(u8, 0x44);
      D_80143BB0 = 1;
      record[2] = 0;
      return 1;
    }
  }
  return 0;
}
