#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell; the original re-materialises its
 * address for every access, so it keeps the symbol form.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

typedef s32 (*ScenarioScena0600WorkByte4Handler)(s32 arg0, s8 arg1, s8 arg2,
                                                 s16 arg3, s16 arg4, u8 arg5,
                                                 u8 arg6);

/* The three in-image handler words at 0x801FE46C: 0x801FD3BC, 0x801FD568 and
 * 0x801FD748, the last being
 * stateCursorDepthResnap. They
 * sit inside the 0x801FE3BC textbin row, after the shared field seeds that end
 * at 0x801FE46B and before the coordinate lookup records at 0x801FE478, and this
 * dispatcher supplies the argument block the three handlers read: no other word
 * of the payload or of the main executable names any of them.
 * @source 0x801FE46C @kind table
 */
extern ScenarioScena0600WorkByte4Handler D_801FE46C[];

/* @source 0x801FD34C
 * @behavior Overlay step dispatch on work byte 0x04 that preserves the caller's
 *           argument block: it loads the work record published at the scratchpad
 *           pointer cell 0x1F800044 once, takes that record's unsigned byte at
 *           offset 0x04, scales it by four and calls the matching entry of the
 *           three-word in-image handler run at 0x801FE46C with all seven
 *           arguments it received. The register arguments pass straight through
 *           after the entry conversions, which sign-extend the second and third
 *           as signed bytes and the fourth as a signed halfword; the fifth, sixth
 *           and seventh are read from the caller's stack slots 0x10, 0x14 and
 *           0x18 as a signed halfword and two unsigned bytes and are re-passed
 *           in the same order. The handler's answer travels back as a signed
 *           byte, so only that low byte reaches the caller. No instruction word
 *           of this payload and no word of the main executable holds 0x801FD34C,
 *           so the overlay loader that steps the run is its caller and the entry
 *           conversions are the only evidence for the argument roles, which stay
 *           address-anchored. The 0x28-byte frame holds the three re-passed stack
 *           arguments at 0x10, 0x14 and 0x18 and keeps $ra at 0x20 across the
 *           indirect call; the handler base reaches $at through the assembler's
 *           symbol-plus-register expansion, and the slots before the scale and
 *           after the table load stay nops.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape notes from the original bytes: the entry conversions are spread through
 * the prologue and the three stack arguments are loaded before $ra is saved,
 * which is the psyq scheduler hoisting the call's operand loads. The table call
 * is the same symbol-plus-register expansion the overlay dispatchers use, taken
 * here with an argument block instead of the bare no-argument call.
 */
s8 forwardArgsHandlerScenario(
    s32 arg0, s8 arg1, s8 arg2, s16 arg3, s16 arg4, u8 arg5, u8 arg6) {
  return D_801FE46C[D_1F800044[4]](arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}
