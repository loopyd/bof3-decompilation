#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800AD0A8
 * @behavior Copies one byte pair of the class rule slot 4 into the 0x118-byte
 *   local panel record selected by `index`, at record bytes 0x6E/0x6F. The pair
 *   and its sign come from battle work byte 0x08: mode 0 publishes class bytes
 *   0x6D/0x6F, mode 1 the negated 0x6D/0x6F, mode 2 the negated 0x6C/0x6E and
 *   mode 3 class bytes 0x6C/0x6E; any other mode leaves the record unchanged.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Matching note: the class rule bytes stay explicit non-volatile fixed
 * addresses. The original encodes each load as a full 32-bit constant
 * displacement (`lui $at,%hi(X)` + `addu at,index,at`), which a declared symbol
 * reference reproduces with the reversed `addu at,at,index` and a volatile view
 * with a pooled `lui`+`ori` base (measured 82.61% and 18.58%). The record side
 * needs the opposite view: two named symbols (`D_801EB712`/`D_801EB713`, no
 * addend) because a symbol plus a 0x6E addend under the 0x118-indexed arm split
 * makes gcc allocate an unused 32-byte frame (measured 82.61%).
 */
void func_800AD0A8(u8 index, u16 class_id) {
  BattleWork *work;

  work = (BattleWork *)g_battle_work;

  switch (work->unk_08) {
  case 0:
    BATTLE_LOCAL_PANEL_UNK_6E(index) = PSX_REF(u8, 0x800E40BDu + (u32)class_id * 0x88u);
    BATTLE_LOCAL_PANEL_UNK_6F(index) = PSX_REF(u8, 0x800E40BFu + (u32)class_id * 0x88u);
    break;
  case 1:
    BATTLE_LOCAL_PANEL_UNK_6E(index) = -PSX_REF(u8, 0x800E40BDu + (u32)class_id * 0x88u);
    BATTLE_LOCAL_PANEL_UNK_6F(index) = PSX_REF(u8, 0x800E40BFu + (u32)class_id * 0x88u);
    break;
  case 2:
    BATTLE_LOCAL_PANEL_UNK_6E(index) = -PSX_REF(u8, 0x800E40BCu + (u32)class_id * 0x88u);
    BATTLE_LOCAL_PANEL_UNK_6F(index) = PSX_REF(u8, 0x800E40BEu + (u32)class_id * 0x88u);
    break;
  case 3:
    BATTLE_LOCAL_PANEL_UNK_6E(index) = PSX_REF(u8, 0x800E40BCu + (u32)class_id * 0x88u);
    BATTLE_LOCAL_PANEL_UNK_6F(index) = PSX_REF(u8, 0x800E40BEu + (u32)class_id * 0x88u);
    break;
  }
}
