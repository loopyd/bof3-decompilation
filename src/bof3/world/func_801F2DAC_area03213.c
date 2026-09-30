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
 */
void func_801F2DAC(void) {
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
