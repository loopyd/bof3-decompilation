#include "bof3/bof3.h"

/* Work-record pointer cell. It is the last word of the overlay data run at
 * 0x801EF9C0 and the word just past the 96-record pool at 0x801EF9F8. The
 * original re-materializes the cell address for each of the two reads in this
 * function (lui $vN,0x1F80 / lw $vN,0x5F8($vN)), so the cell is declared
 * volatile, matching the exact bmagic sibling
 * dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic16603_801EEF34. */
extern u8 *volatile D_801F05F8; /* @source 0x801F05F8 @kind data */

/* Four-entry overlay handler table for work-record byte 2. The code-pointer
 * run starts one word earlier at 0x801EF9C0, which the byte-1 dispatch at
 * 0x801EEEE0 indexes; this function indexes the second entry of the run. */
extern void (*D_801EF9C4[])(void); /* @source 0x801EF9C4 @kind data */

/* Projects the work record's position and installs the resulting transform. */
extern void func_801EF52C(void);

/* Draws the geometry of the active work record. */
extern void func_801EF5F4(void);

/* @source 0x801EEF24
 * @behavior Reads the work-record pointer cell at 0x801F05F8 and uses its
 *           record byte at offset 0x2, scaled by four, to select one entry of
 *           the handler table based at 0x801EF9C4, which it calls with no
 *           arguments. When the handler returns it reads the cell again in a
 *           second load; if the record byte at offset 0x0 and the byte at
 *           offset 0x2 are both non-zero it calls func_801EF52C, then
 *           func_801EF5F4, then PopMatrix. Reads no other state, stores
 *           nothing and returns nothing. The 0x18-byte frame only keeps $ra
 *           across the indirect call.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenDrawAndPopWhenActiveBmagicMagic10603_801EEF24(void) {
    u8 *step_record;
    u8 *draw_record;

    step_record = D_801F05F8;
    D_801EF9C4[step_record[2]]();
    draw_record = D_801F05F8;
    if (draw_record[0] != 0 && draw_record[2] != 0) {
        func_801EF52C();
        func_801EF5F4();
        PopMatrix();
    }
}
