#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u8 func_8014D978(void); /* @source 0x8014D978 */
extern s32 func_801782FC(s32 angle); /* @source 0x801782FC */
extern s32 func_801783C8(s32 angle); /* @source 0x801783C8 */
extern void clearWorkBytesBmagicMagic22503_801F0338(void); /* @source 0x801F0338 */

/* @source 0x801F021C
 * @behavior Calls 0x8014D978, adds 0x800 to work words 0x40 and 0x44,
 * and adds four times the samples from 0x801782FC and 0x801783C8 to work
 * words 0x34 and 0x38. Both sample calls take work word 0x14; the work
 * pointer is reloaded after each call. Decrements work byte 9 and, at zero,
 * decrements byte B of the record at 0x801EB4E0 and clears work bytes 0-4.
 * Callback at table word 0x801F03B8; no arguments or result. Other field
 * roles remain offset-based.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepWorkPositionPairGrowWords40And44CountdownByte9AndClearAtZeroBmagicMagic22503_801F021C(void) {
    u8 *work;
    u8 *work2;
    u8 *work3;
    s32 sample;

    func_8014D978();
    work = g_battle_work;
    *(s32 *)(work + 0x40) += 0x800;
    *(s32 *)(work + 0x44) += 0x800;
    sample = func_801782FC(*(s32 *)(work + 0x14));
    work2 = g_battle_work;
    *(s32 *)(work2 + 0x34) += sample * 4;
    sample = func_801783C8(*(s32 *)(work2 + 0x14));
    work3 = g_battle_work;
    *(s32 *)(work3 + 0x38) += sample * 4;
    if (--work3[9] == 0) {
        D_801EB4E0[0xB]--;
        clearWorkBytesBmagicMagic22503_801F0338();
    }
}
