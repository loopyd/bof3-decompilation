#include "bof3/battle/battle03_internal.h"

/* @source 0x801EAE94 @kind data */
extern u8 countdownEntryXOffsets[];
/* @source 0x801EAE95 @kind data */
extern u8 countdownEntryYOffsets[];

/* @source 0x801E927C
 * @behavior Submits entries zero through four to func_801644D8 using the panel
 * halfwords +4/+6, interleaved byte offsets at 0x801EAE94/0x801EAE95 and half
 * their countdown as position offsets, with both dimensions equal to the byte
 * value 16 minus the countdown and final argument 128. If countdown zero is
 * zero, advances panel state byte +3; otherwise decrements all seven countdowns.
 * @status partial
 * @match 78.26
 * @residual First-loop load scheduling and register allocation differ.
 */
void submitCountdownEntriesAndAdvance(void) {
  u8 index;
  u32 countdown;
  u8 dimension;
  u32 inset;
  u32 x_offset;
  u32 y_offset;
  u32 offset;

  index = 0;
  do {
    offset = index * 2;
    countdown = D_801462FC[index];
    x_offset = countdownEntryXOffsets[offset];
    y_offset = countdownEntryYOffsets[offset];
    inset = countdown >> 1;
    dimension = 16 - countdown;
    func_801644D8(index,
                 (u16)(inset + (*(u16*)&D_80148648[4] + x_offset)),
                 (u16)(inset + (*(u16*)&D_80148648[6] + y_offset)),
                 dimension, dimension, 128);
    index++;
  } while (index < 5);

  if (D_801462FC[0] == 0) {
    D_80148648[3]++;
  } else {
    index = 0;
    do {
      D_801462FC[index]--;
      index++;
    } while (index < 7);
  }
}
