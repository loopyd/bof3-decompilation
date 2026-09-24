#include "bof3/bof3.h"

void func_8015C088(void);
extern s8 D_801448EB;
extern s8 D_801448EC;
extern s8 D_801448ED;

/* @source 0x801F2C04
 * @behavior Initialises this overlay's shared text-window state: calls the helper
 * func_8015C088, then writes 0x2C, 0 and 0x0B to the three consecutive shared bytes at
 * 0x801448EB, 0x801448EC and 0x801448ED.
 * @status exact
 * @match 100.00
 * @residual none
 */
void initArea093State(void) {
  func_8015C088();
  D_801448EB = 0x2C;
  D_801448EC = 0;
  D_801448ED = 0xB;
}
