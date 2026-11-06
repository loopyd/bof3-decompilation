#include "bof3/bof3.h"

typedef void (*BmagicMagic06403Handler)(void);

typedef struct BmagicMagic06403DispatchTable {
  BmagicMagic06403Handler handlers[4];
} BmagicMagic06403DispatchTable;

/*
 * Code pointer table at 0x801EEC40, the table this dispatcher copies:
 * entry 0 = 0x801EF7F8, entry 1 = 0x801F009C, entry 2 = 0x801F0C38,
 * entry 3 = 0x801F0D64.
 */
extern BmagicMagic06403DispatchTable D_801EEC40; /* @source 0x801EEC40 @kind data */

/* @source 0x801EF78C
 * @behavior Dispatches one frame of the overlay work handler from the
 * scratchpad work object: the work-object pointer cell at 0x1F800044 is read
 * before the 0x28-byte frame is created, its state byte at offset 1 selects an
 * entry of the four-entry code pointer table at 0x801EEC40 (0x801EF7F8,
 * 0x801F009C, 0x801F0C38, 0x801F0D64), and that entry is invoked with no
 * arguments. The four table words are copied into this function's own frame
 * first, so the call target is read back from the copy, and the frame only
 * keeps $ra across the indirect call. It reads no other state and writes none.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchWorkByte1HandlerBmagicMagic06403_801EF78C(void) {
  BmagicMagic06403DispatchTable handlers;

  handlers = D_801EEC40;
  handlers.handlers[SPAD_PTR_SLOT(u8, 0x44u)[1]]();
}
