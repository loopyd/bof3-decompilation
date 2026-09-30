#include "bof3/world/area03004_internal.h"

/* @source 0x801DAF18
 * @behavior AREA030 selection-step handler: stores 2 in the shared AREA030
 * state byte D_8014419E, advances the shared selection counter D_80144199,
 * clears the level byte D_801440B1, publishes the y position field y_38 of the
 * active 0x98-byte work record D_80146888[D_801E3208] into D_8014930C, starts
 * the frontend selection effect func_8015D4F8(0, 0, 100, 8), then stores 7 in
 * the shared mode byte and advances byte 3 of the scratch work record
 * published at 0x1F800044.
 * @status partial
 * @match 78.05
 * @residual Same-size (164/164) scheduler/allocation mismatch. The symbol
 * representation, frame (0x18, ra at 0x10), folds and call are exact. First
 * diff at +0x0010: the original folds the counter load right after the
 * D_8014419E store and keeps addiu+sb after the D_801440B1 clear, while the
 * current object hoists the D_801E3208 index load and sinks the counter trio
 * after the 0x98 index chain, reusing v1. 8 clean-C statement, temporary,
 * lifetime and volatility spellings produced this same object, so the residual
 * is not reachable from clean-C source order here. Next untried evidence is an
 * authorized object compiler-profile probe (bin/flag-search) or the bounded
 * permuter, both opt-in and not authorized for this mission.
 */
void func_801DAF18(void) {
    u8 count;
    u8* work;

    D_8014419E = 2;
    count = D_80144199_BYTE;
    D_801440B1 = 0;
    D_80144199_BYTE = count + 1;
    D_8014930C = D_80146888[D_801E3208].y_38;
    func_8015D4F8(0, 0, 100, 8);
    work = D_1F800044;
    modeByte = 7;
    work[3]++;
}
