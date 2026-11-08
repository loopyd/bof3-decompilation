#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads this cell for the
 * byte-8 argument and loads it again after the resource request, so the source
 * keeps the named cell form and evaluates it at both uses.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Readiness poll helper of the main executable; the battle and bmagic lifts
 * declare the same prototype. The original keeps only the returned byte as the
 * ready test, so the declared return type is that byte.
 */
extern u8 func_8014DAEC(void);

/* Shared main-executable resource request helper; the sibling world, boss and
 * bmagic lifts declare the same prototype and pass a resource id.
 */
extern void func_8014D6B8(u32 flag);

/* @source 0x801CFE58
 * @behavior Work-step handler of the scratch work object published at the
 *           scratchpad pointer cell 0x1F800044. Its only in-image reference is
 *           the word 0x801CFE58 at 0x801D08A8, entry 20 of this overlay's
 *           twenty-four-entry code-pointer run at 0x801D0858; the byte-3
 *           sub-dispatcher func_801CF998 indexes that same run from entry 17
 *           (0x801D089C), so work-record byte 3 reading 3 reaches this handler
 *           and no jal in the payload names it. It calls the shared readiness
 *           helper func_8014DAEC and keeps only that helper's returned byte as
 *           a ready test: while the helper reports not ready the handler writes
 *           nothing. Once it reports ready the handler passes the record's
 *           unsigned byte 8 to the shared resource request helper
 *           func_8014D6B8, then re-loads the pointer cell and advances the
 *           record's unsigned byte 3, the work-step index, by one, so the next
 *           dispatch on that byte reaches entry 21, the word 0x801CFEB8 at
 *           0x801D08AC. Takes no arguments and returns nothing; it reads or
 *           writes no other state, and the 0x18-byte frame only keeps the
 *           return address at 0x10 across its two calls. The roles of record
 *           byte 8 and of the requested resource are not established here, so
 *           the name describes the effect. The 96 bytes at 0x801CFE58 are
 *           word-for-word identical to func_801CFC30 of overlay
 *           emi/plchar/plp016/00 and to func_801D0054 of
 *           emi/plchar/plp026/00, whose lifted bodies take the same two
 *           pointer-cell loads.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResourceForWorkByte8ThenAdvanceWorkByte3WhenHelperReadyPlcharPlp05600_801CFE58(
    void) {
  if (func_8014DAEC()) {
    func_8014D6B8(D_1F800044[8]);
    D_1F800044[3]++;
  }
}
