#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell. The original loads it once ahead of
 * frame creation for the route-byte read and again before every store of the
 * body, so the source keeps the named symbol form and evaluates it again at
 * each access.
 * @source 0x1F800044 @kind data
 */
extern u8* D_1F800044;

/* Route step-sign helper of the shared main executable, outside this payload.
 * This target's generated undefined_funcs_auto.txt binds its address as
 * func_801B2D00. It classifies a requested route index against the work route
 * byte 0x08 within a wrapped eight-entry window, answers -1 when the bound
 * stands one to four entries above the request and 1 otherwise, and takes one
 * zero-extended byte argument, so both calls here pass a small immediate.
 */
s32 func_801B2D00(u8 index);

/* @source 0x801D014C
 * @behavior Work byte 3 == 9 step of this overlay that is also entry 0 of the
 *           work-byte-2 dispatch: the word 0x801D014C stands at 0x801D05B0,
 *           entry 19 of this payload's thirty-word code-pointer run 0x801D0564
 *           through 0x801D05D8, which is the base the work-byte-2 dispatcher
 *           0x801D0108 indexes by work byte 2, and it sits 0x24 bytes above the
 *           base 0x801D058C that the byte-3 dispatcher 0x801CEF20 indexes by
 *           work byte 3, so work byte 3 == 9 reaches it. No jal in this payload
 *           names it and the pointer word at 0x801D05B0 is its only in-image
 *           reference, so its prologue and return path alone place the
 *           boundary. The body takes the unsigned route index byte 8 of the
 *           work record read through the scratchpad pointer cell 0x1F800044
 *           and compares its distance from the two candidate route indices 3
 *           and 5: when the distance to 5 is the smaller one it asks the
 *           shared helper func_801B2D00 for the step sign towards 5 and stores
 *           that answer in work byte 0x0B, otherwise it asks for the sign
 *           towards 3 and stores that. Both paths then store the chosen
 *           candidate index in work byte 3, so the two branches share one
 *           store and differ only in the argument and in the constant they
 *           hand to the helper. The body then sets the countdown byte 9 to 2
 *           and increments the step counter byte 2, which re-arms the work
 *           object for the selected route. It takes no arguments, returns
 *           nothing and answers no value. The route byte is loaded once and
 *           both distances are computed from it; the pointer cell is re-read
 *           before every store because a store through the pointer may alias
 *           the cell itself; the first pointer load precedes frame creation,
 *           the 0x18-byte frame only keeps $ra at 0x10 across the calls and
 *           the two paths join on one epilogue. When this seed was written the
 *           0xCC-byte span agreed with the exact lift at plp27a 0x801CE448 in
 *           50 of its 51 words: the one differing word is the intra-body jump
 *           at span offset 0x64, which encodes its own shared-store address
 *           0x801D01DC against 0x801CE4D8, a shift of exactly the 0x1D04
 *           address delta between the two spans, and the same single-word
 *           relation held for the exact lifts at plp278 0x801CF89C, plp025
 *           0x801CF968, plp034 0x801D0094, plp056 0x801CF76C, plp349 0x801CF36C,
 *           plp457 0x801CE664, plp478 0x801CF2C0, plp567 0x801CEA44 and plp578
 *           0x801CF6A0, while plp012 and plp015 0x801CF170 also differ in the
 *           candidate immediates, asking for route 7 instead of 5. Those
 *           siblings are therefore shape evidence and not clones, and their
 *           statement order, operand forms and declaration style are carried
 *           over with this body own literals and offsets.
 * @status exact
 * @match 100.00
 * @residual none
 */
void pickNearerRouteIndexStoreStepSignAndChosenRouteThenRearmWorkPlcharPlp67800_801D014C(
    void) {
  s32 fromThree;
  s32 fromFive;

  fromThree = D_1F800044[8] - 3;
  if (fromThree < 0) {
    fromThree = -fromThree;
  }
  fromFive = D_1F800044[8] - 5;
  if (fromFive < 0) {
    fromFive = -fromFive;
  }
  if (fromFive < fromThree) {
    D_1F800044[0xB] = func_801B2D00(5);
    D_1F800044[3] = 5;
  } else {
    D_1F800044[0xB] = func_801B2D00(3);
    D_1F800044[3] = 3;
  }
  D_1F800044[9] = 2;
  D_1F800044[2] = D_1F800044[2] + 1;
}
