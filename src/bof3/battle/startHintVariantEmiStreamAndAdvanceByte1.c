#include "bof3/battle/battle03_internal.h"

/* Shared hint byte at 0x80145024 whose low seven bits index the EMI stream slot
 * families of initStreamSlot (family 1 base 0x1DB, family 2 base 0x1EE, see
 * func_8016728C); it lives outside this overlay, so it keeps its
 * address-qualified name. */
extern u8 D_80145024;

/* Pointer cells at 0x800B4A64/0x800B4A68 holding the u16 slot-id tables read
 * when the 0x8000 gate of BATTLE_GLOBAL_HALF_62E8 is clear. Both live outside
 * this overlay and stay address-qualified. */
extern u16 *D_800B4A64;
extern u16 *D_800B4A68;

/* @source 0x801E7948
 * @behavior While bit 0x8000 of the shared battle halfword 0x801462E8 is set,
 * starts the EMI stream slot selected by the shared hint byte 0x80145024: with
 * the paired work records reporting ready, hint values 7, 0xD, 0xE and 0xF pick
 * slots 0x201, 0x202, 0x203 and 0x204, or 0x205, 0x206, 0x207 and 0x208 when
 * work byte +0x08 of D_801EB4E0 is two or more; otherwise the hint byte is the
 * index within stream family 0x1DB (work byte +0x08 below two) or family 0x1EE.
 * With that gate word clear the slot is instead the u16 read from the pointer
 * cell D_800B4A64 or D_800B4A68 at that hint index. The selected slot is
 * started through initStreamSlot and scratchpad work byte +0x01 is incremented.
 * @status exact
 * @match 100.00
 * @residual none
 */
void startHintVariantEmiStreamAndAdvanceByte1(void) {
  u32 slot;

  if ((BATTLE_GLOBAL_HALF_62E8 & 0x8000u) != 0u) {
    if (func_801E7B34() != 0u) {
      if (D_80145024 == 7u) {
        if (D_801EB4E0->unk_08 < 2u) {
          slot = 0x201u;
        } else {
          slot = 0x205u;
        }
        func_80161FDC(slot);
      }
      if (D_80145024 == 0xDu) {
        if (D_801EB4E0->unk_08 < 2u) {
          slot = 0x202u;
        } else {
          slot = 0x206u;
        }
        func_80161FDC(slot);
      }
      if (D_80145024 == 0xEu) {
        if (D_801EB4E0->unk_08 < 2u) {
          slot = 0x203u;
        } else {
          slot = 0x207u;
        }
        func_80161FDC(slot);
      }
      if (D_80145024 == 0xFu) {
        if (D_801EB4E0->unk_08 < 2u) {
          slot = 0x204u;
        } else {
          slot = 0x208u;
        }
        func_80161FDC(slot);
      }
    } else {
      if (D_801EB4E0->unk_08 < 2u) {
        func_80161FDC(D_80145024 + 0x1DBu);
      } else {
        func_80161FDC(D_80145024 + 0x1EEu);
      }
    }
  } else {
    if (D_801EB4E0->unk_08 < 2u) {
      func_80161FDC(D_800B4A64[D_80145024]);
    } else {
      func_80161FDC(D_800B4A68[D_80145024]);
    }
  }
  SPAD_PTR_SLOT(u8, 0x44u)[1]++;
}
