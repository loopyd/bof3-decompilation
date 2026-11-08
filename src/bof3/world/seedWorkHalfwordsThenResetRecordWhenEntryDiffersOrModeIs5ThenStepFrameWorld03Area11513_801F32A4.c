#include "bof3/bof3.h"

extern u8* D_1F800044;
extern u16 D_80143B90;
extern u8 D_80143BB0;
extern u16 D_80143F10;

/* Local seed at 0x801F3404: writes 0xA0 and 0x50 to the work halfwords. */
void setWorkHalfwordsWorld03Area11513_801F3404(void);

/* Shared per-frame helper at 0x8014D4E0: the call site passes nothing and
 * consumes no result. */
void func_8014D4E0(void);

/* @source 0x801F32A4
 * @behavior Seeds the work halfwords at record offsets 0x2E and 0x30. Then,
 *           unless the shared state halfword 0x80143B90 reads 1, resets the
 *           work record published at scratchpad pointer slot 0x44 when its
 *           byte 0x0B differs from its byte 0x07, or the shared mode byte
 *           0x80143BB0 reads 5, or byte 0x0B reads 1 while the record word at
 *           0x18 differs from the shared context seed 0x80143F10. The reset
 *           writes 2 to record byte 0x48, 8 to byte 0x09 and 4 to the record
 *           mode byte 0x01, each through a fresh read of the pointer slot.
 *           Finally the shared helper 0x8014D4E0 runs. Takes no arguments,
 *           uses no result and saves only the return address. The 0xC0 bytes
 *           are byte-identical to the exact func_801F32A0 of
 *           emi/world00/area016/13 apart from the low byte of the internal
 *           `jal` word that encodes that overlay's own seed address, so this
 *           body follows the reviewed sibling statement order with this
 *           target seed.
 * @status exact
 * @match 100.00
 * @residual none
 */
void seedWorkHalfwordsThenResetRecordWhenEntryDiffersOrModeIs5ThenStepFrameWorld03Area11513_801F32A4(
    void) {
  u8* record;

  setWorkHalfwordsWorld03Area11513_801F3404();
  if (D_80143B90 != 1) {
    record = D_1F800044;
    if (record[0x0B] != record[0x07] || D_80143BB0 == 5 ||
        (record[0x0B] == 1 && FIELD_REF(u32, record, 0x18) != D_80143F10)) {
      D_1F800044[0x48] = 2;
      D_1F800044[0x09] = 8;
      D_1F800044[0x01] = 4;
    }
  }
  func_8014D4E0();
}
