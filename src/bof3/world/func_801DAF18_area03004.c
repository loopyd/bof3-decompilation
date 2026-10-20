#include "bof3/world/area03004_internal.h"

/* @source 0x801DAF18
 * @behavior AREA030 selection-step handler: stores 2 in the shared AREA030
 * state byte D_8014419E, advances the shared selection counter D_80144199,
 * clears the level byte D_801440B1, publishes the y position field y_38 of the
 * active 0x98-byte work record D_80146888[D_801E3208] into D_8014930C, starts
 * the frontend selection effect func_8015D4F8(0, 0, 100, 8), then stores 7 in
 * the shared mode byte and advances byte 3 of the scratch work record
 * published at 0x1F800044.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * The field-address temporary is load-bearing: taking the address of y_38
 * before the counter store keeps the 0x801E3208 index load and the 0x98 index
 * chain in the head block, so the D_80144199_BYTE load/increment/store stay at
 * their original positions. Folding the subscript into the store statement
 * instead lets the -O2 scheduler sink the counter trio behind the index chain
 * and reuse v1 for it.
 */
void func_801DAF18(void) {
    u8 count;
    u8* work;
    s32* yPos;

    D_8014419E = 2;
    count = D_80144199_BYTE;
    D_801440B1 = 0;
    yPos = &D_80146888[D_801E3208].y_38;
    D_80144199_BYTE = count + 1;
    D_8014930C = *yPos;
    func_8015D4F8(0, 0, 100, 8);
    work = D_1F800044;
    modeByte = 7;
    work[3]++;
}
