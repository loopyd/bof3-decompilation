#include "base/types.h"

/* Shared battle global halfword at 0x801462E8, outside this overlay image. It
 * lives in the main executable, so it keeps its reviewed address-qualified
 * name; Splat binds it through this target generated undefined_syms_auto.txt
 * row, so no target-map row is added here. The exact battle lift func_800AE438
 * and the bmagic lifts that raise its bit 0x0004 declare the same volatile
 * halfword, and this body only tests that bit once. */
extern volatile u16 D_801462E8; /* @source 0x801462E8 @kind unknown */

/* Shared helper at 0x801E5988, outside this overlay image. The bmagic lifts
 * that raise bit 0x0004 of the halfword above invoke it with no argument, and
 * Splat binds the address through this target generated
 * undefined_funcs_auto.txt row. Nothing in this payload establishes a narrower
 * role, so the name stays address-anchored. */
extern void func_801E5988(void); /* @source 0x801E5988 */

/* Shared main-executable helper at 0x800AFD9C, outside this overlay image. The
 * bmagic step body at 0x801EEF2C calls it with no argument beside the shared
 * helper func_800AF764, and Splat binds the address through this target
 * generated undefined_funcs_auto.txt row. Its role is not established by this
 * payload, so the name stays address-anchored. */
extern void func_800AFD9C(void); /* @source 0x800AFD9C */

/* @source 0x801EF2F0
 * @behavior Overlay step handler at word 1 of the two-word code pointer run
 *           based at 0x801EFA84 - word 0 is 0x801EF058 and word 1 is this body
 *           - which the step dispatcher dispatchStateByte1Tablefa84BmagicMagic
 *           08103_801EF014 indexes with work-object byte 0x1, so no jal in this
 *           payload names it. It reads the shared battle global halfword at
 *           0x801462E8 once and tests bit 0x0004: when the bit is set it calls
 *           the shared helper func_801E5988 with no arguments, and when it is
 *           clear it calls the shared main-executable helper func_800AFD9C
 *           with no arguments. Either way it then returns. The 0x18-byte frame
 *           keeps $ra at 0x10 across the one call, the halfword load stays
 *           ahead of the frame creation, and no other state is read or
 *           written. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeHelperWhenFlag4SetElseInvokeMainExecutableHelperBmagicMagic08103_801EF2F0(
    void) {
    if (D_801462E8 & 4) {
        func_801E5988();
    } else {
        func_800AFD9C();
    }
}
