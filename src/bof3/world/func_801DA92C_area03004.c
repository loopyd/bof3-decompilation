#include "bof3/world/area03004_internal.h"

/* @source 0x801DA92C
 * @behavior AREA030 mode-entry step: stores 1 in the shared state byte
 * D_8014419E, increments the shared step counter byte D_8014403E, advances the
 * shared selection counter D_80144199_BYTE, stores 1 in byte 6 of the work
 * record published at the scratchpad cursor 0x1F800044, clears the four
 * D_801E31E0 slot character counters, clears record byte 0x07 and (before the
 * func_8014D8D4(3) call) record byte 0x0B, then increments record byte 3.
 * @status partial
 * @match 65.91
 * @residual Same-size (176/176, 29/44 insns) head-block register-allocation
 * mismatch. Frame (0x18, ra at 0x10), the fold/call shapes, the loop body
 * (sb zero,0(a3) / addiu t0,t0,-1 / bgez t0 / addiu a3,a3,-1), the post-loop
 * store sequence and the record byte-3 increment are exact. First diff at
 * +0x0004: the original materializes the loop counter before the loop pointer
 * and holds the cursor in a0, the D_8014403E base in a1, the shared constant 1
 * in a2, the loop pointer in a3 and the counter in t0, while the current object
 * keeps the cursor in a0 and the loop pointer in a3 but puts the constant in
 * a1, the D_8014403E base in v1 and the counter in a2 (the a1/a2 pair merges
 * both byte temporaries and sinks the counter register). Clean-C levers
 * exhausted: about 36 measured variants over five shape sweeps (22-29%),
 * covering head statement order, named byte temporaries versus direct
 * read-modify-write, declaration initializers, for-init lists, do/while and
 * index forms of the clear loop, and the order of the two cursor stores; all
 * plateau at 29. Next untried evidence is an authorized object compiler-profile
 * probe (bin/flag-search / bin/compiler-variants) or the bounded permuter, both
 * opt-in and not authorized for this mission.
 */
void func_801DA92C(void) {
    u8 step;
    u8 count;
    u8* work;
    u8* p;
    s32 i;

    step = D_8014403E[0];
    count = D_80144199_BYTE;
    D_8014419E = 1;
    D_80144199_BYTE = count + 1;
    D_8014403E[0] = step + 1;
    D_1F800044[6] = 1;
    p = D_801E31E0 + 3;
    for (i = 3; i >= 0; i--) {
        *p-- = 0;
    }
    D_1F800044[7] = 0;
    D_1F800044[0x0B] = 0;
    func_8014D8D4(3);
    work = D_1F800044;
    work[3]++;
}
