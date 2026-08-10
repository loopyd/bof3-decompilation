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
  u8 pad_20[0x14];
  u32 source_34;
  u32 source_38;
  u32 source_3c;
} WorkareaSelectState;

/* @source 0x801F2C48 @behavior copies three workarea fields and advances mode
 * @status exact
 * @match 100.00
 * @residual none; live audit is instruction- and byte-exact.
 */
void copyWorkareaFieldsAndAdvanceMode(void) {
  WorkareaSelectState* state;
  u32* source_3c;
  WorkareaSelectState* loaded_state;
  WorkareaSelectState* mode;
  u32 value_34;
  u32 value_38;
  u32 slot_offset;
  u32 value_3c;

  /*
   * MATCHING_AID: keeping the scratchpad offset in a temporary and the two
   * pointer-cell reads as separate expressions emits the original two
   * lui+lw pairs; direct SPAD_PTR_SLOT CSEs the address through $a3.
   * A bounded permuter run found this clean-C shape; remove if compiler
   * evidence later reproduces the original loads without the temporary.
   * The immediately following live byte-match was exact.
   */
  slot_offset = 0x44u;
  loaded_state = PSX_REF(WorkareaSelectState*, SPAD_BASE + slot_offset);
  state = loaded_state;
  value_34 = state->source_34;
  value_38 = state->source_38;
  source_3c = &state->source_3c;
  value_3c = *source_3c;
  state->timer_09 = 10;
  mode = PSX_REF(WorkareaSelectState*, SPAD_BASE + (u32)(0x40u + 4u));
  state->unk_1c = 0;
  state->saved_0c = value_34;
  state->saved_10 = value_38;
  state->saved_14 = value_3c;
  mode->mode_01++;
}
