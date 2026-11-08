#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell of the shared main executable. The
 * original re-reads the cell after every store of the body, because a store
 * through the pointer may alias the cell itself, so the source keeps the
 * named-symbol form and evaluates it again at every access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Two-argument resource request of the shared EXE image, outside this payload:
 * the shipped body masks its first argument with 0x80, and the AREA030 step
 * initializer calls it as func_8014D6DC(0xA, 2). This target's generated
 * undefined_funcs_auto.txt binds the address. */
void func_8014D6DC(s32 resource, s32 count);

/* Single-argument resource request of the shared EXE image, outside this
 * payload. The plp016 and plp026 work-byte-3 step handlers pass their record
 * byte 8 to it, and this target's generated undefined_funcs_auto.txt binds the
 * address. */
void func_8014D6B8(s32 resource);

/* Work-sequence selector of the shared EXE image, outside this payload: it takes
 * a zero-extended byte sequence id and answers a word this body ignores. The
 * exact sibling
 * selectArrivalSequence41ForRouteThreeAnd40ForRouteFiveAndAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp01200_801CF610
 * of this same target declares the same prototype, and this target's generated
 * undefined_funcs_auto.txt binds the address. */
u32 func_8014D8D4(u8 arg0);

/* @source 0x801CE628
 * @behavior Overlay step of the front-end work object read through the
 *           scratchpad pointer cell 0x1F800044, reached from outside this
 *           payload: the body is entry 1 of the sixteen-word code-pointer run at
 *           0x801CFD80 that the work-byte-2 dispatcher indexes, so it runs
 *           while work byte 0x02 still holds 1. Byte 0x08 is the current route
 *           index of the eight-entry route window, byte 0x03 the chosen
 *           candidate index the re-arm step stores, byte 0x09 the countdown and
 *           byte 0x0B the step sign of that route walk. When the current route
 *           index already equals the chosen index the walk has arrived, so the
 *           body requests the resource named by work byte 0x04 through the
 *           shared helpers: the two-argument form func_8014D6DC with the
 *           resource 0x50 and the count 2 when byte 0x04 already reads 0x50, and
 *           the single-argument form func_8014D6B8 with byte 0x04 otherwise. It
 *           then increments the step counter byte 0x02, which advances the
 *           dispatch to the next entry of the same run. Otherwise it stores the
 *           countdown byte 0x09 one lower, and when that reaches zero puts it
 *           back to 2, adds the step sign byte 0x0B into the route index byte
 *           0x08, masks that index into the eight-entry window with 7 and
 *           selects the sequence of the new route index through func_8014D8D4.
 *           It takes no arguments, returns nothing and answers no value. The
 *           0x18-byte frame only keeps $ra at 0x10 across the calls, the
 *           increment of byte 0x02 issues in the delay slot of the jump to the
 *           epilogue and the constant 2 is materialised in the delay slot of the
 *           branch that skips the expiry path. Work byte 0x04 is only read here,
 *           never written by this payload, so its role stays at its offset. The
 *           0x104-byte body is byte-identical at this address in the plp015,
 *           plp016, plp025, plp026, plp034 and plp056 payloads, whose rows there
 *           are all still `asm`, so those twins supply no source to clone; the
 *           countdown tail from the byte-0x09 read onward is instruction-for-
 *           instruction the one of the exact sibling of this same target at
 *           0x801CF23C, and only the arrival branch differs from it.
 * @status exact
 * @match 100.00
 * @residual none
 */
void requestResourceForWorkByte4ThenAdvanceStepWhenRouteReachedElseTickCountdownAndStepRoutePlcharPlp01200_801CE628(
    void) {
  if (D_1F800044[8] == D_1F800044[3]) {
    if (D_1F800044[4] == 0x50) {
      func_8014D6DC(0x50, 2);
    } else {
      func_8014D6B8(D_1F800044[4]);
    }
    D_1F800044[2] = D_1F800044[2] + 1;
  } else {
    D_1F800044[9] = D_1F800044[9] - 1;
    if (D_1F800044[9] == 0) {
      D_1F800044[9] = 2;
      D_1F800044[8] = D_1F800044[8] + D_1F800044[0xB];
      D_1F800044[8] = D_1F800044[8] & 7;
      func_8014D8D4(D_1F800044[8]);
    }
  }
}
