#include "bof3/world/area03004_internal.h"

/* @behavior increments byte 9 of the work record published at the scratchpad
 * cursor 0x1F800044 and redraws the AREA030 panel at the origin
 * 0x6C - 0x50 * that count through func_801E1758; once the count reaches 5 it
 * restarts it at zero and, while the record byte 6 is clear, advances the shared
 * halfword counter D_80143B92, otherwise advances the shared step byte
 * D_8014403D and clears the record dispatch byte 3; then the dim tile is
 * appended through appendDimTile, and while that restarted count is nonzero the
 * inset panel rectangle 0x12 - 8 * count and the texture row 0x15 - 8 * count
 * are submitted through func_801D9534 and func_8014F800 using the halfword
 * offset-table entry 0x40 on the 0x80010000 main-RAM base.
 * @source 0x801DA2A0
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA2A0(void) {
    u8 *work;
    u8 count;
    u16 *half_counter;
    u8 *step_byte;

    D_1F800044[9]++;
    func_801E1758((s16)(0x6C - D_1F800044[9] * 0x50), 0x56);
    work = D_1F800044;
    if (work[9] == 5) {
        work[9] = 0;
        work = D_1F800044;
        if (work[6] == 0) {
            half_counter = &D_80143B92;
            *half_counter = *half_counter + 1;
        } else {
            step_byte = &D_8014403D;
            *step_byte = *step_byte + 1;
            work[3] = 0;
        }
    }
    appendDimTile();
    count = D_1F800044[9];
    if (count != 0) {
        func_801D9534(0x14, (0x12 - count * 8) & 0xFFFE, 0x118, 0x13, 0);
        func_8014F800(0x1D, 0x15 - D_1F800044[9] * 8, 0, 0xFF,
                      0x80010000u + PSX_PTR(u16, 0x80010000u)[0x40]);
    }
}
