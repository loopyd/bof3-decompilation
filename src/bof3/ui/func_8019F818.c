#include "bof3/ui/game00_internal.h"

extern void func_8019D800(void);
extern void func_8019DC6C(void);

/* @behavior Calls func_8019D800 and then func_8019DC6C in that order. Both
 * callees take no arguments and the caller ignores any result, so no argument
 * register is set up before either jal.
 * @source 0x8019F818
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_8019F818(void)
{
    func_8019D800();
    func_8019DC6C();
}
