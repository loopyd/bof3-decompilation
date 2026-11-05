#include "bof3/battle/battle03_internal.h"

/* @source 0x801EAE94 @kind data */
extern u8 countdownEntryXOffsets[];
/* @source 0x801EAE95 @kind data */
extern u8 countdownEntryYOffsets[];

/* @source 0x801D7A40
 * @behavior Draws one countdown-scaled battle panel entry per battler around
 * the panel anchor halfwords. Each battler entry uses width code 0x80, lowered
 * to 0x40 for the battlers two and three while word +0x128 of the record
 * published at 0x801EBF08 carries bit 0x2. For the battler indices zero through
 * four, skipping the selected battler byte 0x801462F4, it submits through
 * func_801644D8 an entry positioned at the anchor plus the byte offsets at
 * 0x801EAE94/0x801EAE95 less half that battler's countdown byte 0x801462FC and
 * sized countdown plus 0x10 in both dimensions. It then repeats the width code
 * for the selected battler while that byte is below five and, when that
 * battler's countdown is zero, submits the same func_801644D8 entry without the
 * countdown inset and size 0x10; otherwise, and for the extra battlers five and
 * six while their countdown bytes are non-zero, it draws through func_801D7D10
 * the same inset position with countdown plus 0x10 as size.
 * @status partial
 * @match 97.22
 * @residual The extra battlers five and six re-materialise the constant byte
 * pointer 0x80146301 into a scratch register for each read, where the original
 * keeps it in s0 across the intervening call and reads 0(s0) then 1(s0).
 */
void drawBattlerCountdownEntries(s16 anchorX, s16 anchorY) {
  u8 index;
  u32 mode;
  u8 countdown;
  u8* extra;

  mode = 0;
  index = 0;
  do {
    if (index != D_801462F4) {
      mode = 0x80;
      if ((u8)(index - 2) < 2u) {
        if ((FIELD_REF(u32, g_BtlMessageSlot, 0x128u) & 2u) != 0u) {
          mode = 0x40;
        }
      }
      func_801644D8(index,
                    anchorX + countdownEntryXOffsets[index * 2] -
                        (D_801462FC[index] >> 1),
                    anchorY + countdownEntryYOffsets[index * 2] -
                        (D_801462FC[index] >> 1),
                    D_801462FC[index] + 0x10, D_801462FC[index] + 0x10, mode);
    }
    index++;
  } while (index < 5u);

  if (D_801462F4 < 5u) {
    mode = 0x80;
    if ((u8)(D_801462F4 - 2) < 2u) {
      if ((FIELD_REF(u32, g_BtlMessageSlot, 0x128u) & 2u) != 0u) {
        mode = 0x40;
      }
    }
  }

  countdown = D_801462FC[D_801462F4];
  if (countdown == 0u) {
    func_801644D8(D_801462F4,
                  anchorX + countdownEntryXOffsets[D_801462F4 * 2],
                  anchorY + countdownEntryYOffsets[D_801462F4 * 2], 0x10, 0x10,
                  mode);
  } else {
    func_801D7D10(D_801462F4,
                  (u16)(anchorX + countdownEntryXOffsets[D_801462F4 * 2] -
                        (countdown >> 1)),
                  (u16)(anchorY + countdownEntryYOffsets[D_801462F4 * 2] -
                        (countdown >> 1)),
                  (u8)(countdown + 0x10), (u8)(countdown + 0x10), mode);
  }

  extra = &D_801462FC[5];
  countdown = extra[0];
  if (countdown != 0u) {
    func_801D7D10(5,
                  (u16)(anchorX + countdownEntryXOffsets[10] - (countdown >> 1)),
                  (u16)(anchorY + countdownEntryYOffsets[10] - (countdown >> 1)),
                  (u8)(countdown + 0x10), (u8)(countdown + 0x10), 0x80);
  }
  countdown = extra[1];
  if (countdown != 0u) {
    func_801D7D10(6,
                  (u16)(anchorX + countdownEntryXOffsets[12] - (countdown >> 1)),
                  (u16)(anchorY + countdownEntryYOffsets[12] - (countdown >> 1)),
                  (u8)(countdown + 0x10), (u8)(countdown + 0x10), 0x80);
  }
}
