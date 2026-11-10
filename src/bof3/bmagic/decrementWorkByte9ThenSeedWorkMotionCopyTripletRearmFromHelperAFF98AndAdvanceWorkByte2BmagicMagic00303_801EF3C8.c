#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern void func_801DD350(u8 *work); /* @source 0x801DD350 */
extern u8 func_800AFF98(void); /* @source 0x800AFF98 */

/* @source 0x801EF3C8
 * @behavior Decrements scratchpad work byte 9 modulo 256. On zero, reads
 * work words 0x34, 0x38 and 0x3C, seeds words 0xC, 0x14, 0x10 and 0x20
 * with 0x2000, 0x300000, zero and 0xFFF80000, then copies the saved words
 * to 0x18, 0x1C and 0x44. Passes that work pointer to helper 0x801DD350,
 * calls 0x800AFF98 and stores its returned byte in work byte 9. Reloads
 * the work pointer and increments byte 2. The payload table references
 * this entry at offset 4. Takes no arguments and returns nothing; word
 * roles and helper semantics remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9ThenSeedWorkMotionCopyTripletRearmFromHelperAFF98AndAdvanceWorkByte2BmagicMagic00303_801EF3C8(void) {
  u32 *work;
  u32 first;
  u32 second;
  u32 third;
  u8 value;

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
    value = func_800AFF98();
    g_battle_work[9] = value;
    g_battle_work[2]++;
  }
}
