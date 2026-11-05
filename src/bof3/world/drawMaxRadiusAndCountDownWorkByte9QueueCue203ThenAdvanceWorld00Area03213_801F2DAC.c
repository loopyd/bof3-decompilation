#include "bof3/world/area03213_internal.h"

/* @source 0x801F2DAC
 * @behavior Emits the scratch cursor record quad ring through func_801F2F04
 * with the record angle pair at 0x2E/0x30 and the work byte at 9 mapped to the
 * 0x3C/0x3D ring selector, plays cue 0x203 through the shared dispatcher while
 * that byte reads 0xD7, then decrements the work byte and, once it reaches
 * zero, parks it at 0x3C and advances the state byte at 1.
 * @status exact
 * @match 100.00
 * @residual none
 * State 2 of the local handler table at 0x801F3F6C: the ring radius is held at
 * the maximum the grow state reaches (0x3C/0x3D, flickering on work byte 9's
 * low bit) while that byte counts down 0xFF -> 0, and cue 0x203 is queued on
 * the 0xD7 frame.
 */
void drawMaxRadiusAndCountDownWorkByte9QueueCue203ThenAdvanceWorld00Area03213_801F2DAC(
    void) {
  u8 work;

  func_801F2F04(*(s16*)(D_1F800044 + 0x2e), *(s16*)(D_1F800044 + 0x30),
                (D_1F800044[9] & 1) | 0x3c, 0x80, 0);

  if (D_1F800044[9] == 0xd7) {
    func_8015DF18(0x203);
  }

  work = D_1F800044[9];
  work--;
  D_1F800044[9] = work;

  if (work == 0) {
    D_1F800044[9] = 0x3c;
    D_1F800044[1]++;
  }
}
