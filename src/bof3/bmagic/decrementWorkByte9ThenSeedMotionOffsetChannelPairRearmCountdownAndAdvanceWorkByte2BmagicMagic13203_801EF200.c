#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */
extern u32 D_80149308[]; /* @source 0x80149308 @kind unknown */
extern u32 D_8014930C[]; /* @source 0x8014930C @kind unknown */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */

/* @source 0x801EF200
 * @behavior Decrements scratchpad work byte 9 modulo 256. On zero, copies
 * byte 8 from the record published at 0x801EB4E0, seeds work words 0xC
 * and 0x10 with 0xFFF80000 and 0xFFF10000, and calls 0x801DD350 with
 * that work pointer. Reloads work to add words 0xC and 0x10 to the shared
 * channel words at 0x80149308 and 0x8014930C into words 0x34 and 0x38.
 * Seeds words 0xC and 0x10 with 0x8000 and zero, copies published record
 * word 0x3C plus 0x800000 into work word 0x3C, and calls the same helper.
 * Through separate work-pointer reads, sets byte 0xB to 0x1A, byte 9 to
 * 0x2C and byte 0xA to zero. A final read clears word 0x1C, sets word
 * 0x20 to 0x80 and advances byte 2. Entry 1 of the table at 0x801EFD58
 * names this handler. Takes no arguments and returns nothing. Other record,
 * channel and helper roles remain unproven. The byte countdown and channel
 * additions follow the exact magic114/03 handler at 0x801EFE70; each
 * constant and the two helper calls come from this span's original bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSeedMotionOffsetChannelPairRearmCountdownAndAdvanceWorkByte2BmagicMagic13203_801EF200(void) {
  u32 *work;
  u32 *position;
  u32 *step;
  u32 depth;

  if (--g_battle_work[9] == 0) {
    g_battle_work[8] = D_801EB4E0[8];
    work = (u32 *)g_battle_work;
    work[3] = 0xFFF80000;
    work[4] = 0xFFF10000;
    func_801DD350((u8 *)work);
    position = (u32 *)g_battle_work;
    position[13] = D_80149308[0] + position[3];
    position[14] = D_8014930C[0] + position[4];
    depth = *(u32 *)(D_801EB4E0 + 0x3C);
    position[3] = 0x8000;
    position[4] = 0;
    position[15] = depth + 0x800000;
    func_801DD350((u8 *)position);
    g_battle_work[0xB] = 0x1A;
    g_battle_work[9] = 0x2C;
    g_battle_work[0xA] = 0;
    step = (u32 *)g_battle_work;
    step[7] = 0;
    step[8] = 0x80;
    ((u8 *)step)[2]++;
  }
}
