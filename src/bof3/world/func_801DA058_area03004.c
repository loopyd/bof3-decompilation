#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 panel step: decrements byte 9 of the work record published
 * at the scratchpad cursor 0x1F800044 and redraws the panel at the origin
 * 0x50 * count + 0x6C through func_801E1758; once that count reaches zero it
 * arms record byte 6 and advances its dispatch byte 3; then the dim tile is
 * appended through appendDimTile, the inset panel rectangle 0x12 - 8 * count is
 * submitted through func_801D9534 and the texture row 0x15 - 8 * count is drawn
 * through func_8014F800 using the halfword offset-table entry 0x40 on the
 * 0x80010000 main-RAM base.
 * @source 0x801DA058
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA058(void) {
    u8 *work;

    work = D_1F800044;
    work[9] = work[9] - 1;
    func_801E1758((s16)(D_1F800044[9] * 0x50 + 0x6C), 0x56);
    work = D_1F800044;
    if (work[9] == 0) {
        work[6] = 1;
        work = D_1F800044;
        work[3]++;
    }
    appendDimTile();
    func_801D9534(0x14, (0x12 - D_1F800044[9] * 8) & 0xFFFE, 0x118, 0x13, 0);
    func_8014F800(0x1D, 0x15 - D_1F800044[9] * 8, 0, 0xFF,
                  0x80010000u + PSX_PTR(u16, 0x80010000u)[0x40]);
}
