#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */
extern s32 func_8019651C(void *work, s32 x, s32 y, s32 z, s32 mode); /* @source 0x8019651C */
extern s8 D_801F0A78[]; /* @source 0x801F0A78 @kind data */
extern s8 D_801F0A79[]; /* @source 0x801F0A79 @kind data */
extern s8 D_801F0A7A[]; /* @source 0x801F0A7A @kind data */

/* @source 0x801EF7A8
 * @behavior Decrements scratchpad work byte 9 modulo 256. On zero, saves
 * words 0x34, 0x38 and 0x3C, seeds words 0xC, 0x14, 0x10 and 0x20 with
 * 0x2000, 0x300000, zero and 0xFFF80000, copies the saved triplet to
 * 0x18, 0x1C and 0x44, and invokes helper 0x801DD350. Reloads the work
 * pointer at each byte update: sets flag 0x20 in byte 0, writes 3 to byte
 * 0x5C and writes minus sixteen times byte 0xB to bytes 0x5D, 0x5F and
 * 0x5E in that order. Submits the work pointer and the signed-byte triplet
 * at 0x801F0A78 indexed by three times byte 0xB to helper 0x8019651C
 * with mode 1, then reloads work and increments byte 2. The payload
 * references this callback at offset 0x18; it takes no arguments and
 * returns nothing. Work-field roles remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9SeedMotionTripletSetFlag20SubmitIndexedEffectAdvanceWorkByte2BmagicMagic00303_801EF7A8(void) {
  u32 *work;
  u32 first;
  u32 second;
  u32 third;
  u32 index;

  if (--g_battle_work[9] == 0) {
    work = (u32 *)g_battle_work;
    first = work[13];
    second = work[14];
    third = work[15];
    work[3] = 0x2000;
    work[5] = 0x300000;
    work[4] = 0;
    work[8] = 0xFFF80000;
    work[6] = first;
    work[7] = second;
    work[17] = third;
    func_801DD350((u8 *)work);
    g_battle_work[0] |= 0x20;
    g_battle_work[0x5C] = 3;
    g_battle_work[0x5D] = -(g_battle_work[0xB] << 4);
    g_battle_work[0x5F] = -(g_battle_work[0xB] << 4);
    g_battle_work[0x5E] = -(g_battle_work[0xB] << 4);
    index = g_battle_work[0xB] * 3;
    func_8019651C(g_battle_work, D_801F0A78[index], D_801F0A79[index], D_801F0A7A[index], 1);
    g_battle_work[2]++;
  }
}
