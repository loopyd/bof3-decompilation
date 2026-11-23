#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. The original folds the
 * symbolic address into each load (lui $vN,0x1F80 / lw $vN,68($vN)) and
 * re-materialises the cell before every store group, because a store through the
 * loaded pointer may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared main-executable resource request helper, called here with the literal
 * one. The generated undefined_funcs_auto.txt already binds the address, because
 * the remaining assembly span func_801EF130 reaches the same helper, so the
 * address-anchored prototype adds no target-map row. */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 @kind function */

/* @source 0x801EF690
 * @behavior Per-frame work-handler step of this overlay, entry index 4 of the
 *           eight-word code pointer run based at 0x801EF8C4 that the state-byte-2
 *           dispatcher dispatchStateByte2Tablef8c4BmagicMagic11503_801EF098
 *           indexes with work object byte 2 scaled by four; the payload word at
 *           0x801EF8D4 is the only in-image reference to this address, so the
 *           entry is selected through that table and takes no arguments. It
 *           seeds the scratchpad work object published at the pointer cell
 *           0x1F800044, in this order: byte 0x25 = 0x1D, byte 0x26 = 0,
 *           the 16.16 motion pair at 0x40 and 0x44 = 0x10000 with byte 0x48 =
 *           1, byte 0x27 = 0xA0, byte 0x28 = 0, byte 0x24 = 4, the tint bytes
 *           0x5D, 0x5E and 0x5F = 0, mode byte 0x5C = 0, byte 0x2A = 0, byte
 *           0x29 = 4, the halfword at 0x2C = 0 and byte 0x2B = 0. It then
 *           requests resource 1 through func_8014D6B8, the store to byte 0x2B
 *           filling the call delay slot, and advances work byte 2 with a
 *           byte-width read, add and store. The 0x18-byte frame keeps $ra at
 *           0x10; nothing is returned and no other state is read or written.
 *           Each store group reads the cell through its own local: the original
 *           re-materialises the cell before every byte store because a store
 *           through the loaded pointer may alias the cell, while the two word
 *           stores and the byte store at 0x40/0x44/0x48 share one load, and one
 *           local reused for all groups collapses those loads onto a single
 *           register. The 276-byte span holds no byte-identical twin anywhere
 *           under out/binaries: a scan of the other 397 extracted payloads for
 *           these bytes with every jump and jump-and-link immediate word masked
 *           returned no copy, so the statement order above comes from the shape
 *           evidence of the exact siblings seedWorkObjectFromBattleRecordAnd
 *           AdvanceWorkByte2 of magic045, which seeds the same header, motion
 *           pair and tint block in this order beside its own head record copy,
 *           and seedWorkHeaderClearTintGroupAndRequestResourceThenAdvanceWork
 *           Byte2 of magic173, which seeds the same header group and cleared
 *           tint trio, and not from a clone seed. Roles of work bytes
 *           0x24, 0x25, 0x27, 0x29 and 0x2A stay offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateHeaderUnitMotion(
    void) {
  u8 *kind;
  u8 *next;
  u8 *motion;
  u8 *tilt;
  u8 *offset;
  u8 *count;
  u8 *tintR;
  u8 *tintG;
  u8 *tintB;
  u8 *mode;
  u8 *tail;
  u8 *index;
  u8 *pair;
  u8 *step;

  kind = g_battle_work;
  kind[0x25] = 0x1D;
  next = g_battle_work;
  next[0x26] = 0;
  motion = g_battle_work;
  *(u32 *)(motion + 0x40) = 0x10000u;
  *(u32 *)(motion + 0x44) = 0x10000u;
  motion[0x48] = 1;
  tilt = g_battle_work;
  tilt[0x27] = 0xA0;
  offset = g_battle_work;
  offset[0x28] = 0;
  count = g_battle_work;
  count[0x24] = 4;
  tintR = g_battle_work;
  tintR[0x5D] = 0;
  tintG = g_battle_work;
  tintG[0x5E] = 0;
  tintB = g_battle_work;
  tintB[0x5F] = 0;
  mode = g_battle_work;
  mode[0x5C] = 0;
  tail = g_battle_work;
  tail[0x2A] = 0;
  index = g_battle_work;
  index[0x29] = 4;
  pair = g_battle_work;
  *(u16 *)(pair + 0x2C) = 0;
  pair[0x2B] = 0;
  func_8014D6B8(1);
  step = g_battle_work;
  step[2]++;
}
