#include "bof3/bof3.h"

/* Shared battle work pointer cell of the main executable. The original builds
 * the cell address with lui %hi / lw %lo and reaches the byte through the
 * loaded pointer, so the source keeps the pointer form rather than a direct
 * byte access at the constant address.
 */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind data */

/* Readiness poll helper of the main executable; the sibling bmagic lifts
 * declare the same prototype and keep only the returned byte as the ready
 * test.
 */
extern u8 func_8014DAEC(void); /* @source 0x8014DAEC @kind function */

/* Shared main-executable helper the sibling bmagic lifts invoke with no
 * arguments and no return value.
 */
extern void func_801E5988(void); /* @source 0x801E5988 @kind function */

/* @source 0x801EF2A8
 * @behavior Third word of the twelve-byte code-pointer run at 0x801EF7A0
 *           (0x801EF038, 0x801EF0D0, 0x801EF2A8), reached as entry 1 of the
 *           two-entry run based at 0x801EF7A4 that
 *           dispatchStateByte2Tablef7a4BmagicMagic07503_801EF038 selects by
 *           scaling the work record's unsigned byte 2 by four; no jal in this
 *           payload names it, so that run alone reaches it. It calls the
 *           shared readiness helper func_8014DAEC and keeps only that helper's
 *           returned byte as a ready test, so while the helper reports not
 *           ready the handler writes nothing. Once it reports ready the
 *           handler loads the shared battle work pointer cell 0x801EB4E0 of
 *           the main executable, advances the pointed-to unsigned byte 0xB by
 *           one, and then invokes the shared helper func_801E5988 with no
 *           arguments. Takes no arguments and returns nothing; the 0x18-byte
 *           frame only keeps the return address at 0x10 across its two calls.
 *           The role of func_801E5988 and of work byte 0xB are not established
 *           beyond the sibling lifts, so the name describes the effect.
 * @status exact
 * @match 100.00
 * @residual none
 */
void incrementGlobalHelperReady(
    void) {
  u8 *globalWork;

  if (func_8014DAEC()) {
    globalWork = D_801EB4E0;
    globalWork[0xB]++;
    func_801E5988();
  }
}
