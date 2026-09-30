#include "bof3/bof3.h"

extern u32 D_80143E6C;

typedef struct Boss01816Work {
  u8  pad_00[0x38];
  u32 unk_38;
} Boss01816Work;

/* @source 0x800C203C
 * @behavior Overlay handler: advances the cursor word at scratchpad work
 * object +0x38 by 0x200 on every other frame, keyed on the low bit of the
 * shared frame counter at 0x80143E6C.
 * @status exact
 * @match 100.00
 * @residual none
 */
void advanceWorkCursorOnFrameParityBossBoss01816_800C203C(void) {
  Boss01816Work* work = SPAD_PTR_TABLE(Boss01816Work)[0x11];

  work->unk_38 += (D_80143E6C & 1u) << 9;
}
