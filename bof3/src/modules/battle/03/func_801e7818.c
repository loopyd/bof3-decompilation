#include "internal.h"

/* does: dispatches the current saved-preview/result task state through the
 * fixed five-entry handler table.
 * @source: 0x801e7818 FUN_801e7818
 */
void BOF3_NO_SIBLING_CALLS func_801e7818(void) {
  Battle03Handler const* savedPreviewResultTable;
  Battle03Handler        table[5];

  savedPreviewResultTable =
      (Battle03Handler const*)BOF3_BATTLE_SAVED_PREVIEW_RESULT_TABLE;
  table[0] = savedPreviewResultTable[0];
  table[1] = savedPreviewResultTable[1];
  table[2] = savedPreviewResultTable[2];
  table[3] = savedPreviewResultTable[3];
  table[4] = savedPreviewResultTable[4];
  table[BOF3_BATTLE_LOCAL_SCRATCH_PTR->unk_01]();
}
