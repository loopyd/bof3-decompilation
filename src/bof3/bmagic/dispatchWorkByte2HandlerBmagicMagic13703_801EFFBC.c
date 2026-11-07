#include "bof3/bof3.h"

typedef void (*BmagicMagic13703Handler)(void);

typedef struct BmagicMagic13703DispatchTable {
  BmagicMagic13703Handler handlers[3];
} BmagicMagic13703DispatchTable;

/*
 * Code pointer table at 0x801EEC10, the table this dispatcher copies:
 * entry 0 = 0x801F001C, entry 1 = 0x801F007C, entry 2 = 0x801F00D8, the three
 * code blocks that follow this overlay's state-1 dispatcher. The run sits in the
 * overlay's leading data block, which no map row owns, so the reference is bound
 * by the target's generated undefined-symbol table.
 */
extern BmagicMagic13703DispatchTable D_801EEC10; /* @source 0x801EEC10 @kind data */

/* @source 0x801EFFBC
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before frame creation, its work byte at offset 2 selects an entry of the
 * three-entry code pointer table at 0x801EEC10 (0x801F001C, 0x801F007C,
 * 0x801F00D8), and that entry is invoked with no arguments. The three table
 * words are copied into its own 0x28-byte frame first, so the call
 * target is read back from the copy and the selector is not bounds-checked.
 * Entry 2 is the shared helper wrapper at 0x801F00D8. It reads no other state
 * and writes none; the frame only keeps $ra across the indirect call. The
 * 96-byte body is byte-identical to the three-entry dispatcher at 0x801EF1F0 of
 * emi/bmagic/magic046/03.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerBmagicMagic13703_801EFFBC(void) {
  BmagicMagic13703DispatchTable handlers;

  handlers = D_801EEC10;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[2]]();
}
