#include "bof3/ui/shop00_internal.h"

/* Shop command table of this overlay's trailing rodata blob, at payload byte
 * 0x14DC0: five-byte records whose first byte is the number of commands of the
 * group and whose following bytes are command codes into the label table below.
 * The shipped records hold four, four, four, two, two, three, two, two and two
 * codes, so the byte after the last code of each record is unused slack. Only
 * this body reads the table, and it sits inside the rodata tail whose extracted
 * label T_801E5144 carries the segment, so no target-map row is added.
 * @source 0x801E59C0 @kind table */
extern u8 D_801E59C0[];

/* Shop command-label table at payload byte 0x14D64: one string pointer per
 * command code, from "Use" at index 0 through "Quit" at index 0xB, over the
 * label strings of the same rodata tail. The body indexes it with the code byte
 * of the record above and hands the pointer to the width query and to the text
 * emitter.
 * @source 0x801E5964 @kind table */
extern const char* D_801E5964[];

/* The two 0x18-byte sprite-record streams of the same rodata tail, emitted once
 * per cell: the first is the one the selected entry takes, the second is the
 * one every other entry takes. Both are only ever submitted, never modified.
 * @source 0x801E55F4 @kind unknown */
extern const u8 D_801E55F4[];
/* @source 0x801E55DC @kind unknown */
extern const u8 D_801E55DC[];

/* Text emitter of the main executable: the two signed coordinates, the
 * alignment argument, the mask and the caption run as the fifth argument, which
 * the original stores to the outgoing stack slot.
 * @source 0x8014F800 */
extern void func_8014F800(s16 arg0, s16 arg1, s32 arg2, u32 arg3, u32 arg4);

/* Sprite-record stream emitter of the main executable: the two signed
 * coordinates, the stream blob and the flag byte it carries.
 * @source 0x801AF390 */
extern void func_801AF390(s16 arg0, s16 arg1, const u8* arg2, u8 arg3);

/* Command-code mapper of the main executable: it reports the mapped code of the
 * table it is given in the requested mode, and this body tests only the low
 * halfword of that answer.
 * @source 0x8016628C */
extern s32 func_8016628C(s32 table, s32 code, s32 mode);

/* @source 0x801DA77C
 * @behavior shop command row emitter of the SHOP overlay: the group byte
 *           selects one five-byte record of D_801E59C0 whose first byte is the
 *           command count and whose following bytes are command codes, and for
 *           every code the body paints the 0x2D by 0x14 panel at the x argument
 *           plus 0x30 per entry and the row y through func_801AE3F0 with the
 *           shade 0 and the CLUT-bank byte D_80144952, sets the shade 7 for
 *           every entry other than the selection while the selection lies
 *           inside the count, forces that shade for the Ally code 0x12 while
 *           bit 0 of D_80143F02 is clear and for the Chng code 8 while
 *           func_80166140 with 0xF and func_8016628C with 0, 0x58 and 0 both
 *           answer zero, loads the command label D_801E5964[code] once, submits
 *           it through the text emitter func_8014F800 at x plus the 0x30 step
 *           plus a right-aligned four-cell inset and y plus 3 with the mask
 *           0x10, and emits the 0x18-byte sprite-record stream D_801E55F4 for
 *           the selected entry or D_801E55DC for any other entry at x plus the
 *           0x30 step and the sign-extended y, with the flag 0. The count byte
 *           is re-read from D_801E59C0 at each test because the calls can reach
 *           it, the 0x50-byte frame keeps the saved registers and $ra, the
 *           fifth argument is never read, and nothing is returned.
 * @status exact
 * @match 100.00
 * @residual none
 *
 * Shape evidence: one payload compare proved these 0x248 bytes equal the
 * emi/etc/shisu/00 span at 0x801D5C7C and the emi/etc/bate/00 span at
 * 0x801D4CD4 apart from seven words, every one of them a payload-local data
 * address (this table, the label table and the two streams) whose immediate
 * differs by the same 0xBCD8. The three spans are therefore one body, so the
 * shisu copy supplied the statement order and neither copy is a clone source.
 *
 * Representation notes, each the spelling a measured alternative on this span
 * selected rather than a matching aid:
 *   - the record offset is formed into its own word and the loop then carries
 *     it separately, so the early count test indexes D_801E59C0 through the
 *     first word and the original materialises the offset after that branch
 *     (one offset variable measured 125 of 146 instructions);
 *   - the selection byte is widened once into a word, which is the value the
 *     three loop comparisons use; keeping the narrow parameter in them
 *     measured the same size with the record pointer and the selection in each
 *     other's registers (137 of 146);
 *   - the text inset is formed as an unsigned term and added as its own sum;
 *     the plain signed sum lets the compiler fold the constant 6 onto the
 *     coordinate instead of the inset (145 of 146).
 */
void drawShopCommandRow(s16 x, s16 y, u8 group, u8 selected, s32 arg4) {
  const char* label;
  u32 offset;
  u32 n;
  u8 shade;
  u8 i;
  u32 sel;

  n = group * 5;
  i = 0;
  if (D_801E59C0[n] == 0) {
    return;
  }
  offset = n;
  sel = selected;
  do {
    u8* record;

    record = &D_801E59C0[offset];
    func_801AE3F0((u16)(x + i * 0x30), (u16)y, 0x2D, 0x14, 0, D_80144952);
    shade = 0;
    if (sel < D_801E59C0[offset]) {
      shade = (i != sel) ? 7 : 0;
    }
    if (*(record + i + 1) == 0x12) {
      if ((D_80143F02 & 1) == 0) {
        shade = 7;
      }
    }
    if (*(record + i + 1) == 8) {
      if ((u8)func_80166140(0xF) == 0) {
        if ((u16)func_8016628C(0, 0x58, 0) == 0) {
          shade = 7;
        }
      }
    }
    label = D_801E5964[*(record + i + 1)];
    func_8014F800(
        (s16)(x + i * 0x30 + (u32)((4 - (u8)func_801AF20C(label)) * 4 + 6)),
        (s16)(y + 3), shade, 0x10, (u32)label);
    func_801AF390((s16)(x + i * 0x30), (s16)y,
                  (i != sel) ? D_801E55DC : D_801E55F4, 0);
    i++;
  } while (i < D_801E59C0[offset]);
}
