#include "bof3/battle/battle15_internal.h"

extern u8 D_801EB6CA[]; /* @source 0x801EB6CA @kind unknown */
extern u8 D_801EB6CE[]; /* @source 0x801EB6CE @kind unknown */

/* @source 0x800A0000
 * @behavior Selection-action handler stored at 0x800B4878 in the battle
 *   selection-action table D_800B471C. Runs the selection-input refresh pass
 *   func_800A3F28, then addresses the 0x118-byte enemy work record band by the
 *   active battler byte D_80146394 rebased by -3 and applies one raise-once step
 *   to two of its byte pairs. When the byte at D_801EB6CA (record + 0x06) is
 *   non-zero and bit 0 of the flag byte at record + 0x07 is clear, it sets that
 *   bit and adds 2 to the value byte. When the byte at D_801EB6CE (record + 0x0A)
 *   is non-zero and bit 0 of the flag byte at record + 0x0B is clear, it sets
 *   that bit and adds 1 to the value byte. Both value bytes are then clamped to
 *   7, so a value of 8 or more is stored back as 7. The battler byte is read
 *   once and the rebased index is kept in a byte local, so the original re-emits
 *   the 0x118 scale chain and the index truncation at each site instead of
 *   re-reading the selection byte. The two raise-once blocks are the same step
 *   over the two byte pairs and differ only in the value added on the store; the
 *   clamp pass carries its own product, because the record offset it computes is
 *   a separate quantity from the pair offsets. That clamp product is written as
 *   one statement per shift so GCC 2.7.2 builds the same five-step multiply and
 *   keeps the product in the register the assembled payload uses.
 * @status exact
 * @match 100.00
 * @residual none
 */
void addToFlaggedEnemyWorkBytesAndClampToSeven(void)
{
    u8 index;
    u32 offset;
    u32 offset3;
    u8 flags;
    u8 value;
    u8 flags2;
    u8 value2;

    func_800A3F28();
    index = D_80146394 - 3;
    offset = (u32)index * 0x118;
    if (*(volatile u8 *)(D_801EB6CA + offset) != 0) {
        flags = *(volatile u8 *)(D_801EB6CA + offset + 1);
        if (!(flags & 1)) {
            value = *(volatile u8 *)(D_801EB6CA + offset);
            *(volatile u8 *)(D_801EB6CA + offset + 1) = flags | 1;
            *(volatile u8 *)(D_801EB6CA + offset) = value + 2;
        }
    }
    offset = (u32)index * 0x118;
    if (*(volatile u8 *)(D_801EB6CE + offset) != 0) {
        flags2 = *(volatile u8 *)(D_801EB6CE + offset + 1);
        if (!(flags2 & 1)) {
            value2 = *(volatile u8 *)(D_801EB6CE + offset);
            *(volatile u8 *)(D_801EB6CE + offset + 1) = flags2 | 1;
            *(volatile u8 *)(D_801EB6CE + offset) = value2 + 1;
        }
    }
    offset3 = (u32)index << 3;
    offset3 = offset3 + (u32)index;
    offset3 <<= 2;
    offset3 -= (u32)index;
    offset3 <<= 3;
    if (*(volatile u8 *)(D_801EB6CA + offset3) >= 8) {
        *(volatile u8 *)(D_801EB6CA + offset3) = 7;
    }
    if (*(volatile u8 *)(D_801EB6CE + offset3) >= 8) {
        *(volatile u8 *)(D_801EB6CE + offset3) = 7;
    }
}
