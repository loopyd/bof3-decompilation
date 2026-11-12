#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7E1C
 * @behavior shop slot price list of the eight-row shop panel: it draws the 0x9E
 *           by 0x70 shaded rectangle at (x + 3, y + 3) through func_801AE3F0
 *           with the shade 0 and the caller's flag byte as the trailing
 *           argument, its frame through func_801DD350 at (x, y) with the
 *           constants 0x12 and 0xD, and then one row per shop slot. Every row
 *           draws the icon handle func_801502D0(row + 0xDE) through
 *           func_8014F800 at x + 8 with the 0xFF mask and, at x + 0x70, the
 *           amount this function formats into the shared text buffer
 *           D_80145AD4 with the EMI-local format string D_801D0CDC
 *           (the payload holds "%3dZ"): ten times the +6 byte of the field
 *           record that the record index byte D_80145FCC[arg0 * 0x140]
 *           selects, which is the same amount the confirm step
 *           drawShopSlotListThenChargeOnConfirm subtracts from the main-RAM
 *           value D_80144F50. It sets the buffer's fourth byte to 0x60 before
 *           submitting the buffer through func_8014FF0C. A row whose bit in
 *           that record's +0x19 mask byte D_80144981 is set, or whose index
 *           equals the caller's selected row byte, draws its icon with the
 *           shade 2; every other row uses the row shade, which is 7 when
 *           D_80144F50 is below the row amount or that mask byte is nonzero and
 *           otherwise 7 or 0 as the caller's flag byte is nonzero or zero. Rows
 *           step 13 pixels down the panel and all eight slots always draw.
 * @status partial
 * @match 97.16
 * @residual instruction and byte counts match (141 instructions, 564 bytes)
 *           and 137 instructions are identical. The original recomputes the
 *           loop counter's andi 0xFF zero-extension in the icon arm's branch
 *           delay slot for the func_801502D0 argument, so its shift and its
 *           selected-row test keep the first copy in t1 while the argument
 *           takes a second one in a0; the build reuses one copy for all three
 *           uses, which leaves a nop in that delay slot and moves the shift's
 *           mask into a0. A second counter local, explicit (i & 0xFF) masks,
 *           a hand-nested arm shape and a shared handle local each measured
 *           the same 137 of 141.
 */
/* The loop counter is a byte, which is why every use of it is zero-extended
 * with andi 0xFF and why the loop test is an unsigned sltiu against 8. The
 * field-record offset is a loop-local because the original recomputes the
 * 164-byte stride product from the record byte in the loop preheader and then
 * indexes the mask table through that value, forming the table base with
 * lui/addu per access instead of keeping a pointer. */
void drawShopSlotPriceList(u8 arg0, u16 arg1, u16 arg2, u8 arg3, u8 arg4) {
  u8 i;
  u8 record;
  u16 amount;
  u16 y;
  u8 shade;

  func_801AE3F0((u16)(arg1 + 3), (u16)(arg2 + 3), 0x9E, 0x70, 0, arg3);
  func_801DD350((s16)arg1, (s16)arg2, 0x12, 0xD);
  record = D_80145FCC[arg0 * 0x140];
  amount = D_80144968[record * 164 + 6] * 10;
  y = arg2 + 6;
  if ((u32)D_80144F50 < amount || D_80144981[record * 164] != 0) {
    shade = 7;
  } else {
    shade = arg3 ? 7 : 0;
  }
  for (i = 0; i < 8; i++) {
    s32 offset;

    offset = record * 164;
    if (((D_80144981[offset] >> i) & 1) || i == arg4) {
      func_8014F800((s16)(arg1 + 8), (s16)y, 2, 0xFF,
                    func_801502D0(i + 0xDE));
    } else {
      func_8014F800((s16)(arg1 + 8), (s16)y, shade, 0xFF,
                    func_801502D0(i + 0xDE));
    }
    sprintf((char*)D_80145AD4, D_801D0CDC, amount);
    D_80145AD4[3] = 0x60;
    func_8014FF0C((s16)(arg1 + 0x70), (s16)y, shade, D_80145AD4);
    y += 0xD;
  }
}
