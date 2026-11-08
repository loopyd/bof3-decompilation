#include "bof3/bof3.h"

extern s8 D_80146870;  /* @source 0x80146870 @kind data */
extern u32 D_8014686C; /* @source 0x8014686C @kind data */

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_801F39AC(void);

/* @source 0x801F3964
 * @behavior Frame handler whose address is published in the local pointer
 * word 0x801F41CC of this overlay's own tail data run. It reads the shared
 * signed scenario byte D_80146870 with lb and compares it against 0x0E: any
 * other value goes straight to the activator call. Only for scenario 0x0E
 * does it read the shared scenario progress word D_8014686C with lw and run
 * the shared two-stage wait 0x8015B5D4 over it with id 5; a nonzero return
 * means the check has not completed (the sibling scenario handler
 * 0x801FD164 reads the same call as a pending wait), and in that one case the
 * handler returns without touching anything, while a zero return still calls
 * the activator. The activator func_801F39AC is the local record path of this
 * overlay: it selects one of the three record banks based at D_801F43D8 from
 * the byte at 0x80144982 together with the ids 0x77 and 0x0A of the same
 * shared wait, then walks six records of stride 0x11 and calls func_801A4D54
 * for every index that the random helper 0x8015CA48 does not report as 0xFF.
 * The 0x18-byte frame keeps only $ra at 0x10, the $ra store and the constant
 * 5 are scheduled into the branch and jal delay slots, and the word cell
 * 0x8014686C is loaded only on the scenario 0x0E path.
 * @status exact
 * @match 100.00
 * @residual none
 */
void activateRecordsUnlessScenarioIs0EAndWait5PendingWorld04Area19113_801F3964(
    void) {
  if (D_80146870 != 0x0E || func_8015B5D4(D_8014686C, 5) == 0) {
    func_801F39AC();
  }
}
