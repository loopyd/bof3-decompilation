#include "bof3/bof3.h"

extern u8 *D_80148648;

/* Battle-end panel handler this shim selects: the overlay's next function at
 * 0x801EFE5C, which drives the panel through func_801F039C, func_801EF928 and
 * the overlay's 0x801EEC40 string run. Still an address-anchored asm function,
 * so it is declared here rather than renamed. */
extern void func_801EFE5C(void);

/* @source 0x801EFE10
 * @behavior Battle-end panel state dispatch through a local handler table:
 *   reads the object published at 0x80148648, takes its byte +0x03 (the
 *   PanelTask state field of include/panel/task.h, the same byte
 *   advanceStateByte3BatlEnd00_801F07C0 increments), scales it by four and
 *   calls the entry at that index of the two-word table built on the stack,
 *   with no arguments. Only entry 0 is seeded (with func_801EFE5C); the index
 *   is not bounded, exactly as in the original. The 0x20-byte frame holds the
 *   table at 0x10($sp) and $ra at 0x18($sp), and the selected handler's $v0 is
 *   left in place by the epilogue, so this shim's result is whatever that
 *   handler returns. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPanelStateLocalTableBatlEnd00_801EFE10(void) {
  void (*table[2])(void);

  table[0] = func_801EFE5C;
  table[((u8 *)D_80148648)[3]]();
}
