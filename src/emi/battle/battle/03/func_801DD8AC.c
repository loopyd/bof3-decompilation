#include "internal.h"

/* @behavior copies the current local battler's visible values and masked flags into
 * the template record selected by byte `0x13c`.
 * @source 0x801DD8AC
 */
void func_801DD8AC(u32 arg0) {
  u32 local_offset;
  u32 product;
  u16 flags;

  local_offset = arg0 & 0xffu;
  product = local_offset << 2;
  product += local_offset;
  local_offset = product << 6;
  if ((BATTLE_LOCAL_ABS_BYTE_5E90(arg0) & 1u) != 0u) {
    BATTLE_TEMPLATE_ABS_HALF_497C(BATTLE_LOCAL_ABS_BYTE_5FCC(arg0)) =
        BATTLE_LOCAL_ABS_HALF_5F18(arg0);
    BATTLE_TEMPLATE_ABS_HALF_497E(BATTLE_LOCAL_ABS_BYTE_5FCC(arg0)) =
        BATTLE_LOCAL_ABS_HALF_5F1A(arg0);
    BATTLE_TEMPLATE_ABS_BYTE_4980(BATTLE_LOCAL_ABS_BYTE_5FCC(arg0)) =
        BATTLE_LOCAL_ABS_BYTE_5F1C(arg0);
    flags = BATTLE_LOCAL_ABS_HALF_5F10(arg0) & 0x60a0u;
    BATTLE_LOCAL_ABS_HALF_5F10(arg0) = flags;
    BATTLE_TEMPLATE_ABS_HALF_4974(BATTLE_LOCAL_ABS_BYTE_5FCC(arg0)) = flags;
  }
}
