#include "bof3/bof3.h"
#include "panel/task.h"

/* The overlay's panel handler run at 0x801EEC28: six handler pointers, of
 * which only entries 0, 1, 4 and 5 are populated (0x801EFE10, 0x801F0650,
 * 0x801F0768, 0x801F097C); entries 2 and 3 are zero words. This shim copies
 * the run into its frame before indexing it. Declared here for its address. */
typedef struct PanelHandlerRunBatlEnd00 {
  void (*handlers[6])(void);
} PanelHandlerRunBatlEnd00;

extern PanelHandlerRunBatlEnd00 D_801EEC28; /* @source 0x801EEC28 @kind table */

/* The shared panel task root cell published at 0x80148648: this shim indexes
 * byte +0x02 of that object, the panel task byte, the same byte the battle
 * overlay dispatchers index. */
extern PanelTask *D_80148648;

/* @source 0x801EFD98
 * @behavior Battle-end panel task dispatch through a local copy of the
 *   overlay's panel handler run: copies the six handler words at 0x801EEC28
 *   into the frame at 0x10($sp), then reads the object published at
 *   0x80148648, takes its byte +0x02 (the PanelTask task byte of
 *   include/panel/task.h, the byte the battle overlay dispatchers index),
 *   scales it by four, adds the stack pointer and calls the entry at that
 *   index with no arguments. The index is not bounded, so the two zero entries
 *   of the run are reachable exactly as in the original. The 0x30-byte frame
 *   holds the run at 0x10($sp) and $ra at 0x28($sp), and this span loads the
 *   panel task cell before the frame adjustment, so that cell load leads the
 *   body. The selected handler's $v0 is left in place by the epilogue, so this
 *   shim's result is whatever that handler returns. Takes no arguments.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchPanelTaskTableEc28BatlEnd00_801EFD98(void) {
  PanelHandlerRunBatlEnd00 handlers;
  PanelTask *task;

  task = D_80148648;
  handlers = D_801EEC28;
  handlers.handlers[task->unk_00[2]]();
}
