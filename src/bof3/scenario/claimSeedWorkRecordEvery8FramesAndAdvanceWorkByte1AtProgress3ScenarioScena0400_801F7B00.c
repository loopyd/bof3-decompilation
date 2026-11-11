#include "bof3/bof3.h"

extern s32 D_80143E6C;
extern s32 D_80146954;
extern s32 D_80146958;
extern s32 D_8014695C;
extern u8 g_ScenarioProgress;

u8* findFreeWorkRecordByte25ScenarioScena0400_801F7FF8(void);
void func_801F7EC0(void);

/* @source 0x801F7B00
 * @behavior Entry 1 of this overlay's eight-word work-object handler run at
 *           0x801FA1E4 ({0x801F7AC8, 0x801F7B00, 0x801F7BC0, 0x801F8124,
 *           0x801F9D48, 0x801F9DB4, 0x801F9DAC, 0x801FA010}), reached only
 *           through the indirect call in
 *           dispatchWorkObjectHandlerScenarioScena0400_801F7A84 (0x801F7A84),
 *           which indexes that run with byte 0x1 of the scratch work object
 *           published at pointer slot 0x1F800044.
 *           On frames whose shared frame counter D_80143E6C reads zero in its
 *           low three bits it claims one free record through the work-table
 *           scan findFreeWorkRecordByte25ScenarioScena0400_801F7FF8
 *           (jal at 0x801F7B18) and seeds that record from the shared saved
 *           triplet: +0x00 takes D_80146954, +0x04 takes D_80146958, +0x08
 *           takes the arithmetic right shift by 8 of D_8014695C, +0x10, +0x14
 *           and +0x18 take zero, +0x20 takes 0x800, and the claim marker 0x80
 *           lands in bytes 0x24 and 0x25, the byte the free-record scan tests.
 *           After the seed block it runs the overlay's record stepper
 *           func_801F7EC0 (jal at 0x801F7B78, no arguments) and then, only when
 *           the shared scenario progress byte g_ScenarioProgress reads 3, it
 *           re-reads the scratch work-object cell and increments work byte 1,
 *           so the next frame dispatches entry 2 at 0x801F7BC0.
 *           The counter load is hoisted above the frame allocation and the
 *           frame's single saved word is $ra at 0x20, so the 0x28-byte frame
 *           only covers the two calls.
 *
 * Name evidence: the only image word holding 0x801F7B00 is offset 0x801FA1E8,
 *           word 1 of the handler run at 0x801FA1E4, so no jal names this body
 *           and the dispatch shape is the naming route rather than missing
 *           evidence. The seed role comes from the claim call and the stored
 *           words, the every-eight-frames gate from the D_80143E6C mask, and
 *           the work-byte advance from the byte this body increments, which is
 *           the same cell the exact entry 0
 *           (clearWorkTableByte25AndAdvanceWorkByte1ScenarioScena0400_801F7AC8)
 *           increments on its way to this entry.
 *
 * Shape evidence: statement order, operand forms and the `*(s32*)(record + N)`
 *           spelling follow the exact sibling lifted from the same shared
 *           triplet, and the tail's byte-increment spelling follows the exact
 *           in-target entry 0 whose original bytes carry the same
 *           `lw 0x44` / `lbu 1` / `addiu` / `sb 1` sequence with the same two
 *           load-delay nops. This function's own offsets and literals come from
 *           its original bytes, and the seed group keeps the +0x08 copy third,
 *           which is where the original loads D_8014695C and where it
 *           materialises its 0x800 constant in that load's delay slot. The
 *           declared scratch array carries no access: it reproduces the 0x10
 *           bytes of frame the original reserves beyond $ra. The clone lever is
 *           declined on a masked payload scan: with the two jal immediate words
 *           masked, these 0xC0 bytes occur at no other offset of the 396
 *           payloads under out/binaries.
 * @status exact
 * @match 100.00
 * @residual none
 */
void claimSeedWorkRecordEvery8FramesAndAdvanceWorkByte1AtProgress3ScenarioScena0400_801F7B00(
    void) {
  /* Reserved frame only; no access and no use. Reconstruction choice. */
  u8 scratch[0x10];
  u8* record;
  u8* work;

  if ((D_80143E6C & 7) == 0) {
    record = findFreeWorkRecordByte25ScenarioScena0400_801F7FF8();
    if (record != 0) {
      *(s32*)(record + 0x00) = D_80146954;
      *(s32*)(record + 0x04) = D_80146958;
      *(s32*)(record + 0x08) = D_8014695C >> 8;
      *(s32*)(record + 0x10) = 0;
      *(s32*)(record + 0x14) = 0;
      *(s32*)(record + 0x18) = 0;
      record[0x24] = 0x80;
      *(s32*)(record + 0x20) = 0x800;
      record[0x25] = 0x80;
    }
  }

  func_801F7EC0();

  if (g_ScenarioProgress == 3) {
    work = SPAD_PTR_SLOT(u8, 0x44u);
    work[1]++;
  }
}
