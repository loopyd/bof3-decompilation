#include "bof3/battle/battle15_internal.h"

/* @source 0x800A36F0
 * @behavior Clears the status-word bits named by the input mask on the battler
 *   selected by the index byte, then, when the mask asks for the 0x20 bit and
 *   the battler still carries it, clears that bit and copies the three words of
 *   the battler's 0x78-byte defence row over the three words at the head of its
 *   action record. The record base is staged in the scratchpad work cell across
 *   the two helper calls, and the stored status word is read back and forwarded
 *   to 0x801DDAB4 before that cell is restored.
 * @status exact
 * @match 100.00
 * @residual none
 */
u16 func_800A36F0(u8 battler_index, u16 flags)
{
  u16 status;
  u16 applied;
  u8 *saved_work;

  if (battler_index < 3u) {
    status = D_80145EC4[battler_index].status;
  } else {
    status = D_801EB664[battler_index - 3].status;
  }

  if (flags & 0x4000u) {
    status &= 0xBFFFu;
  }
  if (flags & 0x80u) {
    status &= 0xFF7Fu;
  }
  if (flags & 0x1u) {
    status &= 0xFFFEu;
  }
  if (flags & 0x2u) {
    status &= 0xFFFDu;
  }
  if (flags & 0x4u) {
    status &= 0xFFFBu;
  }
  if (flags & 0x40u) {
    status &= 0xFFBFu;
  }
  if (flags & 0x8u) {
    status &= 0xFFF7u;
  }
  if (flags & 0x10u) {
    status &= 0xFFEFu;
  }
  if (flags & 0x800u) {
    status &= 0xF7FFu;
  }

  if ((flags & 0x20u) && (status & 0x20u)) {
    status &= 0xFFDFu;
    if (battler_index < 3u) {
      D_80145EC4[battler_index].unk_00 = D_801EC364[battler_index].unk_00;
      D_80145EC4[battler_index].unk_04 = D_801EC364[battler_index].unk_04;
      D_80145EC4[battler_index].unk_08 = D_801EC364[battler_index].unk_08;
    } else {
      D_801EB664[battler_index - 3].unk_00 = D_801EC364[battler_index].unk_00;
      D_801EB664[battler_index - 3].unk_04 = D_801EC364[battler_index].unk_04;
      D_801EB664[battler_index - 3].unk_08 = D_801EC364[battler_index].unk_08;
    }
  }

  if (battler_index < 3u) {
    func_80196718((u8 *)D_80145E90 + battler_index * 0x140u);
    saved_work = g_battle_work;
    D_80145EC4[battler_index].status = status;
    applied = D_80145EC4[battler_index].status;
    g_battle_work = (u8 *)D_80145E90 + battler_index * 0x140u;
  } else {
    func_80196718(D_801EB2E8 + battler_index * 0x118u);
    saved_work = g_battle_work;
    D_801EB664[battler_index - 3].status = status;
    applied = D_801EB664[battler_index - 3].status;
    g_battle_work = D_801EB2E8 + battler_index * 0x118u;
  }
  func_801DDAB4(applied);
  g_battle_work = saved_work;

  return status;
}
