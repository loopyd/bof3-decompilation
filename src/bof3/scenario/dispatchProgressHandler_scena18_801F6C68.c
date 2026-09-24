#include "bof3/bof3.h"

extern s8 D_80146874;
extern u32 D_801F6D78[];

/* @source 0x801F6C68
 * @behavior Dispatches this overlay's per-frame handler named by the signed progress
 * byte D_80146874 through the overlay's own four-entry handler table at 0x801F6D78,
 * invoking the selected entry with no arguments; it reads no other state and writes none.
 */
void dispatchProgressHandler_scena18_801F6C68(void) {
  ((void (*)(void))D_801F6D78[(s8)D_80146874])();
}
