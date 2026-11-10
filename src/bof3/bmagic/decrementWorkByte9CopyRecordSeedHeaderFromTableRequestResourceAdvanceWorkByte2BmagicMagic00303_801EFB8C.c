#include "bof3/bof3.h"

extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */
extern u8 *D_8014638C; /* @source 0x8014638C @kind data */
extern u8 D_801F0B04[]; /* @source 0x801F0B04 @kind data */
extern void func_800AF764(void); /* @source 0x800AF764 */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 */

/* @source 0x801EFB8C
 * @behavior Decrements work byte 9 modulo 256. On zero, copies byte 8 and
 * words 0x34, 0x38 and 0x3C from the record at 0x8014638C into work, then
 * invokes helper 0x800AF764. Seeds work bytes 0x29, 0x25, 0x26 and 0x24
 * with 0, 0x1D, 0 and 0x84; clears byte 0x2B, halfword 0x2C and byte
 * 0x28. Sets byte 0x27 to table 0x801F0B04 indexed by byte 0xB plus
 * 0xB0, and byte 0x2A to 1 or 0 according to bit 0 of byte 4. Requests
 * the resource indexed by work byte 0xB through helper 0x8014D6B8, then
 * advances byte 2. Reloads the scratchpad work pointer for each store
 * group. The frame-local handler pair in 0x801EFAE0 selects this callback
 * by work byte 2. Takes no arguments and returns nothing; field roles
 * remain offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void decrementWorkByte9CopyRecordSeedHeaderFromTableRequestResourceAdvanceWorkByte2BmagicMagic00303_801EFB8C(void) {
  u8 *work;
  u8 countdown;
  u8 *source;
  u8 *copyByte;
  s32 *copy;
  u8 *index;
  u8 *header;
  u8 *next;
  u8 *kind;
  u8 *pair;
  u8 *offset;
  u8 *tilt;
  u8 *tail;
  u8 *clear;
  u8 *step;

  work = g_battle_work;
  countdown = work[9] - 1;
  work[9] = countdown;
  if (countdown == 0) {
    source = D_8014638C;
    copyByte = g_battle_work;
    copyByte[8] = source[8];
    copy = (s32 *)g_battle_work;
    copy[13] = ((s32 *)source)[13];
    copy[14] = ((s32 *)source)[14];
    copy[15] = ((s32 *)source)[15];
    func_800AF764();
    index = g_battle_work;
    index[0x29] = 0;
    header = g_battle_work;
    header[0x25] = 0x1D;
    next = g_battle_work;
    next[0x26] = 0;
    kind = g_battle_work;
    kind[0x24] = 0x84;
    pair = g_battle_work;
    pair[0x2B] = 0;
    offset = g_battle_work;
    *(u16 *)(pair + 0x2C) = 0;
    offset[0x28] = 0;
    tilt = g_battle_work;
    tilt[0x27] = D_801F0B04[tilt[0xB]] + 0xB0;
    tail = g_battle_work;
    if (tail[4] & 1) {
      tail[0x2A] = 1;
    } else {
      tail[0x2A] = 0;
    }
    clear = g_battle_work;
    func_8014D6B8(clear[0xB]);
    step = g_battle_work;
    step[2]++;
  }
}
