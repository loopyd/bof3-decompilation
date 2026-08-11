#include "bof3/ui/commu00_internal.h"

/* @source 0x801F0F08
 * @behavior activates a scratch task and selects its label from the battle-age band table
 * @status exact
 */
void activateScratchTaskWithBandLabel(u8 source_index, u8 task_index, u8 record_kind_index) {
  volatile s8 *scratch;
  u32 elapsed;
  u32 band;
  u32 offset;

  scratch = SPAD_PTR_SLOT(volatile s8, 0x44);
  scratch[6] = 1;

  elapsed = D_8014502C - COMMU00_ACTIVE_RECORDS[source_index & 0xFF].progress_anchor;
  if (elapsed < 20) {
    band = 0;
  } else if (elapsed < 40) {
    band = 1;
  } else {
    band = 2;
  }

  offset = (task_index & 0xFF) * 76;
  FIELD_REF(u16, taskLabelWords, offset * 2) =
      taskLabelBandTable[band][variantRotation[record_kind_index & 0xFF]];
}
