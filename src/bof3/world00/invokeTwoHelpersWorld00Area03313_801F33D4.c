#include "bof3/bof3.h"

void func_801F33FC(void);
void func_801F35C0(void);

/* @source 0x801F33D4
 * @behavior Thin overlay wrapper: calls the helpers at 0x801F33FC and 0x801F35C0 in sequence and returns.
 * @status exact
 * @match 100.00
 * @residual none
 */
void invokeTwoHelpersWorld00Area03313_801F33D4(void) {
  func_801F33FC();
  func_801F35C0();
}
