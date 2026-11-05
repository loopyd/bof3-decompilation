#include "bof3/bof3.h"

extern u8 D_80146865;
extern u8 D_80146866;

/* @source 0x801F33C0
 * @behavior Selects work state 23 when area status byte 0x80146865 is 0xFF
 * or mode byte 0x80146866 is 0x80. Otherwise clears the area status and work
 * state byte 1. Both paths then clear work byte 2, reloading the work pointer
 * from scratchpad cell 0x1F800044 after the state store. Takes no arguments
 * and returns nothing.
 * @status partial
 * @match 83.33
 * @residual Success-arm work pointer and value 23 use exchanged registers.
 */
void selectScratchState23OrClearAreaStatusWorld03Area13413_801F33C0(void) {
    u8 *status;

    status = &D_80146865;
    if (*status == 0xFF || D_80146866 == 0x80) {
        u8 *selectedWork;
        u8 value;

        selectedWork = SPAD_PTR_SLOT(u8, 0x44);
        value = 23;
        selectedWork[1] = value;
    } else {
        u8 *clearedWork;

        clearedWork = SPAD_PTR_SLOT(u8, 0x44);
        *status = 0;
        clearedWork[1] = 0;
    }
    SPAD_PTR_SLOT(u8, 0x44)[2] = 0;
}
