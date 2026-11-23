#include "bof3/bof3.h"

/* Scratchpad work-object pointer cell, shared by this overlay's dispatchers and
 * handlers and bound at 0x1F800044 by this target's map. The original folds the
 * symbolic address into each load (lui $vN,0x1F80 / lw $vN,68($vN)) and
 * re-materialises the cell before every store group, because a store through the
 * loaded pointer may alias the cell itself. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer in main RAM, outside this overlay image. The
 * generated undefined_syms_auto.txt already binds the address because the
 * remaining assembly spans func_801EF27C and func_801EF5C4 read it, so the name
 * needs no target-map row. Words 0x34 and 0x38 read here are the position pair
 * and halfword 0x3E the record height, so the pointer stays address-anchored at
 * record width. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Shared main-executable resource request helper. The generated
 * undefined_funcs_auto.txt already binds the address, so the address-anchored
 * prototype adds no target-map row. */
extern void func_8014D6B8(u32 flag); /* @source 0x8014D6B8 @kind function */

/* @source 0x801EF130
 * @behavior Per-frame work-handler step of this overlay, entry index 0 of the
 *           eight-word code pointer run based at 0x801EF8C4 that the state-byte-2
 *           dispatcher dispatchStateByte2Tablef8c4BmagicMagic11503_801EF098
 *           indexes with work object byte 2 scaled by four; no jal word of the
 *           payload targets this address, so the entry is selected through that
 *           table and takes no arguments. It first copies shared battle record
 *           state into the scratchpad work object published at the pointer cell
 *           0x1F800044: record word 0x34 to work word 0x34, record word 0x38 to
 *           work word 0x38, and record halfword 0x3E plus 0x2C0 to work halfword
 *           0x3E. It then seeds the same work header the sibling entry
 *           seedWorkHeaderAndUnitMotionPairClearTintGroupRequestResource1Then
 *           AdvanceWorkByte2BmagicMagic11503_801EF690 seeds, in this order: byte
 *           0x25 = 0x1D, byte 0x26 = 0, the 16.16 motion pair at 0x40 and 0x44 =
 *           0x10000 with byte 0x48 = 1, byte 0x27 = 0xA0, byte 0x28 = 0, byte
 *           0x24 = 4, the tint bytes 0x5D, 0x5E and 0x5F = 0, mode byte 0x5C =
 *           0, byte 0x2A = 0, byte 0x29 = 4, the halfword at 0x2C = 0 and byte
 *           0x2B = 0. It requests resource 0 through func_8014D6B8, the store to
 *           byte 0x2B filling the call delay slot, sets work byte 9 to 0xFF and
 *           advances work byte 2 with a byte-width read, add and store. The
 *           0x18-byte frame keeps $ra at 0x10; nothing is returned. Each store
 *           group reads the cell through its own local: the two record copies
 *           share the first two cell loads, the record halfword store and the
 *           byte 0x26 store split across the next two, and the original
 *           re-materialises the cell before every later byte store because a
 *           store through the loaded pointer may alias the cell, while the two
 *           word stores and the byte store at 0x40/0x44/0x48 share one load.
 *           The record pointer load and the first work pointer load precede the
 *           frame, since neither needs the stack. The 332-byte span holds no
 *           byte-identical twin: its sibling entry 0x801EF690 seeds the same
 *           header and tint group but carries neither the leading record copy
 *           nor the byte 9 store and requests resource 1, so that sibling
 *           supplies the statement order of the shared region only. Roles of
 *           work bytes 0x24, 0x25, 0x27, 0x29, 0x2A and 0x2C stay
 *           offset-anchored.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHeaderFromBattleRecordClearTintGroupRequestResource0SetWorkByte9ThenAdvanceWorkByte2BmagicMagic11503_801EF130(
    void) {
  u8 *record;
  u8 *work;
  u16 height;
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
  u8 *marker;
  u8 *step;

  record = D_801EB4E0;
  work = g_battle_work;
  *(u32 *)(work + 0x34) = *(u32 *)(record + 0x34);
  *(u32 *)(work + 0x38) = *(u32 *)(record + 0x38);
  height = *(u16 *)(record + 0x3E);
  work[0x25] = 0x1D;
  next = g_battle_work;
  *(u16 *)(work + 0x3E) = height + 0x2C0;
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
  func_8014D6B8(0);
  marker = g_battle_work;
  marker[9] = 0xFF;
  step = g_battle_work;
  step[2]++;
}
