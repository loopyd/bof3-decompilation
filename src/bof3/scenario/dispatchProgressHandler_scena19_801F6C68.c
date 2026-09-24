#include "bof3/bof3.h"

extern s8 D_80146874;
extern u32 D_801F6DDC[];

/* @source 0x801F6C68
 * @behavior Invokes this overlay's frame handler selected by the signed scenario
 * progress byte D_80146874, calling the entry at that index in the overlay's own
 * handler table at 0x801F6DDC with no arguments; the table holds four code
 * pointers (0x801F6CA4, 0x801F6D50, 0x801F6D68, 0x801F6DB4) in the overlay's last
 * 16 bytes, and this function reads no other state and writes none.
 */
void dispatchProgressHandler_scena19_801F6C68(void) {
  ((void (*)(void))D_801F6DDC[(s8)D_80146874])();
}
