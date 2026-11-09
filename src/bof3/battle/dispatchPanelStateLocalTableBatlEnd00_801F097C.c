#include "bof3/bof3.h"

extern u8 *D_80148648;

/* Panel-state handler seeded as entry 0: increments the shared panel state byte
 * (byte +0x03 of the object published at 0x80148648), which is the byte this
 * shim indexes. Declared here for its address. */
extern void advanceStateByte3BatlEnd00_801F09D4(void);

/* Panel-state handler seeded as entry 1: the overlay function at 0x801F09F4,
 * which formats the battle-end text row into the buffer at 0x80149118 and
 * hands it to the panel draw primitives. Its role stays address-anchored, so
 * it is declared here for its address. */
extern void func_801F09F4(void);

/* @source 0x801F097C
 * @behavior Battle-end panel state dispatch through a two-entry local handler
 *   table, the index-5 companion of
 *   dispatchPanelStateLocalTableBatlEnd00_801F0768: seeds a two-word table on
 *   the stack at 0x10($sp) with entry 0 =
 *   advanceStateByte3BatlEnd00_801F09D4 and entry 1 = func_801F09F4, then
 *   reads the object published at 0x80148648, takes its byte +0x03 (the
 *   PanelTask state field of include/panel/task.h), scales it by four, adds the
 *   stack pointer and calls the entry at that index with no arguments. The
 *   index is not bounded, exactly as in the original. This span is the word at
 *   0x801EEC3C, the last of the six panel handler pointers that func_801EFD98
 *   copies from 0x801EEC28 and indexes by byte +0x02 of the same object. The
 *   0x20-byte frame holds the table at 0x10($sp) and $ra at 0x18($sp); this
 *   span loads the state cell before seeding the table, so the cell is bound to
 *   a local pointer ahead of the seeds. The selected handler's $v0 is left in
 *   place by the epilogue, so this shim's result is whatever that handler
 *   returns. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPanelStateLocalTableBatlEnd00_801F097C(void) {
  void (*table[2])(void);
  u8 *task;

  do {
    task = (u8 *)D_80148648;
  } while (0);
  table[0] = advanceStateByte3BatlEnd00_801F09D4;
  table[1] = func_801F09F4;
  table[task[3]]();
}
