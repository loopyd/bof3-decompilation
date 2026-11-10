#include "bof3/bof3.h"

extern u8 *D_801F20DC; /* @source 0x801F20DC @kind data */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern s32 func_801782FC(s32 phase); /* @source 0x801782FC @kind function */
extern void clearRecordBytes0Through4BmagicMagic07403_801F0440(void); /* @source 0x801F0440 @kind function */

/* @source 0x801EFB00
 * @behavior Handler selected by record byte 2 equal to 2 from the three-word
 *           table at 0x801EEC1C. Increments the record halfword at 0xC and
 *           publishes its low six bits shifted left six to scratchpad 0x6.
 *           Samples 0x801782FC at that phase, adds the signed sample times 24
 *           shifted right twelve to halfword 0x8, and stores halfword 0x20.
 *           Adds halfword 0xA into halfword 0x22. Every fourth shared frame,
 *           decrements byte 6. Decrements byte 5 on every call; at zero,
 *           decrements byte 0xB of the record at 0x801EB4E0 and clears this
 *           work record's bytes 0 through 4. Takes no arguments, returns
 *           nothing. Record fields retain their offsets because their roles
 *           are not established.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceRecordHalfwordCAndAccumulateSampleIntoHalfwords20And22ThenDecrementByte6EveryFourthFrameDecrementByte5ClearRecordAtZeroBmagicMagic07403_801EFB00(void) {
  u16 phase;
  s32 sample;
  u8 count;

  phase = ((u16 *)D_801F20DC)[6] + 1;
  ((u16 *)D_801F20DC)[6] = phase;
  SPAD_REF(u16, 0x6u) = (phase & 0x3F) << 6;
  sample = func_801782FC((phase & 0x3F) << 6);
  ((u16 *)D_801F20DC)[0x10] = ((u16 *)D_801F20DC)[4] + ((sample * 0x18) >> 12);
  ((u16 *)D_801F20DC)[0x11] += ((u16 *)D_801F20DC)[5];
  if ((D_80143E6C & 3u) == 0) {
    D_801F20DC[6]--;
  }
  count = D_801F20DC[5] - 1;
  D_801F20DC[5] = count;
  if (count == 0) {
    D_801EB4E0[0x0B]--;
    clearRecordBytes0Through4BmagicMagic07403_801F0440();
  }
}
