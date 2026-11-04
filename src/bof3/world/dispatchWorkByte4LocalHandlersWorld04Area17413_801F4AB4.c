#include "bof3/bof3.h"

extern void func_801F4B0C(void);
extern void func_801F4C68(void);

/* @source 0x801F4AB4
 * @behavior Calls one of two local handlers, func_801F4B0C and func_801F4C68,
 * selected by byte +4 of the work record published at scratchpad slot 0x44.
 * Builds the handler array on the stack and passes no arguments. The selector
 * is unchecked, as in the original. Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte4LocalHandlersWorld04Area17413_801F4AB4(void) {
  void (*handlers[2])(void) = {func_801F4B0C, func_801F4C68};

  handlers[SPAD_PTR_SLOT(u8, 0x44)[4]]();
}
