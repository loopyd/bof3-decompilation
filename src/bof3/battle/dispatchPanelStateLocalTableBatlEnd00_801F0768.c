#include "bof3/bof3.h"

extern u8 *D_80148648;

/* Panel-state handler seeded as entry 0: increments the shared panel state byte
 * (byte +0x03 of the object published at 0x80148648), which is the byte this
 * shim indexes. Declared here for its address. */
extern void advanceStateByte3BatlEnd00_801F07C0(void);

/* Panel-state handler seeded as entry 1: the overlay function at 0x801F07E0,
 * which builds the battle-end panel entries from the battler count and the
 * battle work records. Its role stays address-anchored, so it is declared here
 * for its address. */
extern void func_801F07E0(void);

/* @source 0x801F0768
 * @behavior Battle-end panel state dispatch through a two-entry local handler
 *   table: seeds a two-word table on the stack at 0x10($sp) with entry 0 =
 *   advanceStateByte3BatlEnd00_801F07C0 and entry 1 = func_801F07E0, then reads
 *   the object published at 0x80148648, takes its byte +0x03 (the PanelTask
 *   state field of include/panel/task.h), scales it by four, adds the stack
 *   pointer and calls the entry at that index with no arguments. The index is
 *   not bounded, exactly as in the original. The 0x20-byte frame holds the table
 *   at 0x10($sp) and $ra at 0x18($sp), and this span stores $ra immediately
 *   after the frame adjustment, so the state cell load follows that store:
 *   binding the cell to a local pointer in a one-shot loop body reproduces that
 *   schedule, while the direct expression form put the $ra store between the two
 *   seeds. The selected handler's $v0 is left in place by the epilogue, so this
 *   shim's result is whatever that handler returns. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPanelStateLocalTableBatlEnd00_801F0768(void) {
  void (*table[2])(void);
  u8 *task;

  do {
    task = (u8 *)D_80148648;
  } while (0);
  table[0] = advanceStateByte3BatlEnd00_801F07C0;
  table[1] = func_801F07E0;
  table[task[3]]();
}
