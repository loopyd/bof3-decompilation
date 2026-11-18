#include "bof3/bof3.h"

typedef void (*BmagicMagic06403WorkByte2Handler)(void);

typedef struct BmagicMagic06403WorkByte2HandlerTable {
  BmagicMagic06403WorkByte2Handler handlers[3];
} BmagicMagic06403WorkByte2HandlerTable;

/*
 * Code pointer table at 0x801EEC70, the three-entry work byte 2 handler table
 * this dispatcher copies: entry 0 = 0x801F0CBC, entry 1 = 0x801F0D14 and
 * entry 2 = 0x801F0D34, all function starts of this overlay. It sits in the
 * overlay leading data block, so the reference is bound by the generated
 * undefined-symbol table.
 */
extern BmagicMagic06403WorkByte2HandlerTable D_801EEC70; /* @source 0x801EEC70 @kind data */

/* Scratchpad work-object pointer cell. The original re-materialises this cell
 * for the dispatch index and again after the indirect call, because the called
 * handler may rewrite it, so it is read through the named target symbol
 * instead of the constant-address accessor. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* File-local prototype: the shared main-executable service func_8014D290 is
 * declared per caller, not in a shared header; this caller passes no
 * arguments and uses no result. */
extern void func_8014D290(void); /* @source 0x8014D290 @kind function */

/* @source 0x801F0C38
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object. It is reached as entry 2 of the four-entry code
 * pointer table at 0x801EEC40 (0x801EF7F8, 0x801F009C, 0x801F0C38,
 * 0x801F0D64). The work-object pointer cell at 0x1F800044 is read before the
 * 0x28-byte frame is created, and that object's unsigned byte at offset 2
 * selects an entry of the three-entry work byte 2 handler table at 0x801EEC70
 * (0x801F0CBC, 0x801F0D14, 0x801F0D34) with no bounds test. The three table
 * words are copied into this frame at 0x10 first, so the call target is read
 * back from the copy at 0x10($sp), and the entry is invoked with no arguments.
 * After the handler returns it re-reads the pointer cell and, when the work
 * object's byte at offset 0 is non-zero, invokes the shared main-executable
 * service func_8014D290. It takes no arguments and returns nothing; the frame
 * keeps $ra at 0x20. It reads and writes no other state.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte2HandlerThenInvokeSharedServiceWhenWorkByte0SetBmagicMagic06403_801F0C38(
    void) {
  BmagicMagic06403WorkByte2HandlerTable handlers;

  handlers = D_801EEC70;
  handlers.handlers[g_battle_work[2]]();
  if (g_battle_work[0] != 0u) {
    func_8014D290();
  }
}
