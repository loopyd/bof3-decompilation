#include "bof3/bof3.h"

/* First byte of the six-byte per-mode record run of the main executable that
 * the shop and game targets index as a table base; this overlay writes 0xFF
 * over all six bytes to invalidate both three-byte entries.
 * @source 0x80144F5A @kind data
 */
extern u8 D_80144F5A;

/* @source 0x80144F5B @kind data */
extern u8 D_80144F5B;

/* @source 0x80144F5C @kind data */
extern u8 D_80144F5C;

/* @source 0x80144F5D @kind data */
extern u8 D_80144F5D;

/* @source 0x80144F5E @kind data */
extern u8 D_80144F5E;

/* Last byte of the same six-byte per-mode record run: 0x80144F5F ends the
 * second of its two three-byte entries.
 * @source 0x80144F5F @kind data
 */
extern u8 D_80144F5F;

/* Shared live work-record count byte of the main executable, which the sibling
 * armEffectGroupThenClearWorkRecordsAndResetEffectSlots family clears as well.
 * @source 0x80146254 @kind data
 */
extern u8 D_80146254;

/* Overlay effect-group arm helper of the main executable, outside this payload:
 * it re-arms the effect group for the three ids its first three arguments name
 * with the zeroed fourth argument.
 * @source 0x80166E88 @kind function
 */
extern void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/* Per-id effect teardown helper of the main executable, outside this payload.
 * @source 0x801C187C @kind function
 */
extern void func_801C187C(s32 arg0);

/* @source 0x801FCE6C
 * @behavior Word 1 of the overlay's four-word no-argument handler run at
 * 0x801FD038 ({0x801FCE18, 0x801FCE6C, 0x801FCEDC, 0x801FCEFC}), which
 * dispatchRoute63CoordinateLookupHandlerScenarioScena0300_801FCDB4
 * (0x801FCDB4) indexes with the signed-byte answer of the resident route
 * lookup func_801A7AD8 and calls after its gate, discarding the handler's
 * answer. That pointer word is this span's only in-image reference: no `jal`
 * or `j` word of the payload targets 0x801FCE6C, so it takes no arguments.
 *
 * It re-arms the overlay effect group for the ids 0, 1 and 2 through
 * func_80166E88(0, 1, 2, 0) - $a0 zeroed, $a1=1 and $a2=2 materialised before
 * the jal and $a3 zeroed in that call's delay slot - writes 0xFF into the six
 * consecutive shared bytes D_80144F5A through D_80144F5F, which invalidates
 * both three-byte entries of the per-mode record run those cells begin,
 * clears the shared live work-record count byte D_80146254, and finally runs
 * the per-id effect teardown func_801C187C for id 0, its argument materialised
 * in that jal's delay slot. It reads no memory of its own, consumes no result
 * and returns nothing: the body ends on `jr $ra` with no value setup.
 *
 * The 0x70 (112) bytes use a 0x18-byte frame that keeps only $ra at 0x10
 * across the two calls, the single 0xFF is materialised once in $v0 and reused
 * by all six stores, each of those stores re-materialises the 0x8014 page in
 * $at on its own, and the zero store writes $zero directly, so every stored
 * value is a literal.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupInvalidateModeRecordEntriesClearCountAndResetEffectSlot0ScenarioScena0300_801FCE6C(void) {
  func_80166E88(0, 1, 2, 0);
  D_80144F5A = 0xFF;
  D_80144F5B = 0xFF;
  D_80144F5C = 0xFF;
  D_80144F5D = 0xFF;
  D_80144F5E = 0xFF;
  D_80144F5F = 0xFF;
  D_80146254 = 0;
  func_801C187C(0);
}
