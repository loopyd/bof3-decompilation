#include "bof3/bof3.h"

extern u8 D_801F68A0;
extern u8 D_801F689C;

/* @source 0x801F310C
 * @behavior Clear byte 9 of the scratchpad work record, reload its pointer,
 * set work-state byte 1 to 1, set the shade byte at 0x801F68A0 to 0x0F,
 * and clear the shade selector at 0x801F689C. Takes no arguments and returns
 * nothing. This is entry 0 of the handler table at 0x801F4DFC; entry 1 at
 * 0x801F3148 consumes the shade for the red components of a POLY_G4.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearWorkByte9SelectWorkState1AndShade0FWorld04Area18813_801F310C(void) {
    u8** slots;

    slots = SPAD_PTR_TABLE(u8);
    slots[0x11][9] = 0;
    slots[0x11][1] = 1;
    D_801F68A0 = 0x0F;
    D_801F689C = 0;
}
