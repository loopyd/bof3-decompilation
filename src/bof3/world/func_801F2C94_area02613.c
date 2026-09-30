#include "bof3/world/area02613_internal.h"

typedef struct WorkareaSelectState {
  u8 unk_00;
  u8 mode_01;
  u8 pad_02[7];
  u8 timer_09;
  u8 pad_0a[2];
  u32 saved_0c;
  u32 saved_10;
  u32 saved_14;
  u8 pad_18[4];
  u32 unk_1c;
} WorkareaSelectState;

/* @source 0x801F2C94
 * @behavior services the scratchpad work cursor: hands its +0xC block and its
 * +0x1C word to the local record dispatcher, advances the +0x1C word by 0x1199,
 * and when the +0x9 timer counts down to zero posts cue 0x20D and advances the
 * +0x1 mode byte.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F2C94(void) {
  WorkareaSelectState* state;

  state = (WorkareaSelectState*)D_1F800044;
  func_801F2D5C(&state->saved_0c, state->unk_1c);
  state->unk_1c += 0x1199u;

  if (--((WorkareaSelectState*)D_1F800044)->timer_09 == 0) {
    func_8015DF18(0x20D);
    ((WorkareaSelectState*)D_1F800044)->mode_01++;
  }
}
