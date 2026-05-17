#include "internal.h"

typedef struct World00Area016Overlay801f0000 {
  u8                    unk_0000[0x512c];
  World00Area016Handler state_table_03[1];
} World00Area016Overlay801f0000;

/* does: dispatches through the second local handler table selected by
 * scratchpad state byte `0x03`.
 * @source: 0x801f368c FUN_801f368c
 */
void func_801f368c(void) {
  World00Area016Scratch*         scratch;
  World00Area016Overlay801f0000* overlay;

  scratch = WORLD00_AREA016_SCRATCH_PTR;
  overlay = (World00Area016Overlay801f0000*)0x801f0000;
  overlay->state_table_03[scratch->state_03]();
}
