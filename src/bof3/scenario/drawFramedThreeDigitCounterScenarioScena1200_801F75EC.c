#include "bof3/bof3.h"

/* Frame builder of this overlay at 0x801F765C, the next subsegment: its 0x54C
 * bytes are identical, by payload cmp, to the SHISU frame builder at 0x801D6090
 * and to six other overlay copies - 0x801DAB90 of emi/etc/shop/00, 0x801D1E54
 * of emi/etc/sisyou/00, 0x801D50E8 of emi/etc/bate/00, 0x801D8A40 of
 * emi/etc/commu02/08 and 0x801F3D88 of emi/world00/area008/13 - so it takes a
 * rectangle in the first four words and one scratch argument word. This body
 * only calls it, with the five constants below.
 * @source 0x801F765C @kind function */
extern void func_801F765C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);

/* Line-text drawer of the main executable at 0x8014FF0C: it takes the pen
 * position, a colour word and the text buffer, and is the same callee the
 * shared overlay UI rows draw their formatted values through. This payload
 * only calls it.
 * @source 0x8014FF0C @kind function */
extern void func_8014FF0C(s16 arg0, s16 arg1, s32 arg2, const void* arg3);

extern int sprintf(char* buffer, const char* format, ...);

/* Shared sprintf work buffer of the main executable, outside this EMI payload:
 * the formatted counter is written here and then handed to the line-text
 * drawer as the buffer address.
 * @source 0x80145AD4 @kind data */
extern u8 D_80145AD4[];

/* Format string "%03d" of this overlay's leading data blob, at payload byte 4:
 * the three-digit wide field this body formats its counter with.
 * @source 0x801F6C04 @kind data */
extern const char D_801F6C04[];

/* Byte view of the overlay-local counter halfword at the last payload halfword
 * 0x801FD288: the low byte of that halfword is the 0..0xC7 counter the
 * per-frame handler seedRandomSignAndAdvanceWorkByte2ScenarioScena1200_801F71FC
 * signs with bit 15, and it is the byte this body reads back with lbu.
 * @source 0x801FD288 @kind data */
extern u8 D_801FD288;

/* @source 0x801F75EC
 * @behavior Overlay counter readout, called once per frame from
 *           drawThreeDigitCounterThenResetWorkAreaUnlessFrontEndMode2OrCue4ScenarioScena1200_801F759C:
 *           that handler is entry 2 of the code-pointer run based at
 *           0x801FD178 and holds the only in-image reference to this span, the
 *           jal at 0x801F75A4, while no pointer word anywhere in the payload
 *           names 0x801F75EC. The body first arms a textured frame through
 *           func_801F765C at the rectangle origin (0x8A, 0x1E), width 0x2D,
 *           height 0x14 and scratch argument 1; then it formats the low byte of
 *           the overlay halfword D_801FD288 - read with lbu, so zero-extended -
 *           through sprintf with this overlay's own "%03d" format at payload
 *           byte 4 (0x801F6C04) into the shared text buffer D_80145AD4; then it
 *           draws that buffer at (0x95, 0x22) with colour 0 through
 *           func_8014FF0C, which lands the three digits inside the frame it just
 *           armed, so this call paints the framed three-digit counter. It takes
 *           no arguments, returns nothing and writes no state of its own: the
 *           only stores are the two saved-register slots and sprintf's own
 *           writes into the shared buffer. The 0x20-byte frame keeps $ra and
 *           $s0, and $s0 holds the one materialisation of the buffer address
 *           the body makes, which both the sprintf buffer argument and the
 *           drawer's fourth argument read. The name records the proven
 *           behaviour and the address anchor keeps it target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void drawFramedThreeDigitCounterScenarioScena1200_801F75EC(void) {
  u8* buffer;

  func_801F765C(0x8A, 0x1E, 0x2D, 0x14, 1);
  buffer = D_80145AD4;
  sprintf((char*)buffer, D_801F6C04, D_801FD288);
  func_8014FF0C(0x95, 0x22, 0, buffer);
}
