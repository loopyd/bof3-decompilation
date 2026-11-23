#include "bof3/bof3.h"

/* Shared per-frame counter in main RAM; its low two bits select one step of
 * this handler's four-frame signed cycle.
 * @source 0x80143E6C @kind data
 */
extern s32 D_80143E6C;

/* Shared status byte that receives the mode constant 2.
 * @source 0x80149333 @kind data
 */
extern u8 D_80149333;

/* Shared halfword counter this handler advances.
 * @source 0x8014932A @kind data
 */
extern u16 D_8014932A;

/* Four bytes 00 01 00 FF, that is 0, 1, 0 and -1 read as signed steps. The
 * row sits inside the reviewed data span T_801FDE34 (0x801FDE34 through
 * 0x801FDFCB), behind that span's three-word code-pointer run and its halfword
 * records, and no image word points at it: this handler is its only reader.
 * @source 0x801FDE88 @kind table
 */
extern u8 D_801FDE88[];

/* @source 0x801FCFB4
 * @behavior Leaf step handler of this overlay: it takes the low two bits of the
 * shared per-frame counter D_80143E6C as an index into the four-entry signed
 * step row at 0x801FDE88 (0x801FDE88 through 0x801FDE8B = 0, 1, 0, -1),
 * publishes mode 2 in the shared status byte D_80149333, then adds four times
 * that signed step to the shared halfword counter D_8014932A, wrapping modulo
 * 65536. It reads only D_80143E6C and the step row, writes D_80149333 and
 * D_8014932A, takes no arguments, returns nothing, and keeps no stack frame and
 * no saved register. Two in-image jal sites reach it, 0x801FCF74 and 0x801FCF90
 * inside func_801FC2D0, each taken once the shared sub-state byte D_80146875 has
 * grown to 0x20 and then to 0x27, and no image word anywhere in the payload
 * holds the address. The 0x4C bytes are instruction-for-instruction identical
 * to the exact sibling
 * addSignedTableStepToHalfword8014932AScenarioScena0900_801FC590 at 0x801FC590
 * except for the counter read, a halfword at 0x80146876 there and the word at
 * 0x80143E6C here, and for the step-row immediate; the scena14/00 sibling at
 * 0x801F8800 walks the same counter and row but scales by a byte argument
 * instead of this body's literal four, so it supplied only the shape.
 * @status exact
 * @match 100.00
 * @residual none
 */
void fourTimesSignedTable(
    void) {
  u8 step;

  step = D_801FDE88[D_80143E6C & 3];
  D_80149333 = 2;
  D_8014932A = (u16)(D_8014932A + (s32)(s8)step * 4);
}
