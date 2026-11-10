#include "bof3/bof3.h"

typedef void (*BmagicMagic07403FlaggedHandler)(void);
typedef struct BmagicMagic07403FlaggedTable {
  BmagicMagic07403FlaggedHandler handlers[3];
} BmagicMagic07403FlaggedTable;
extern BmagicMagic07403FlaggedTable D_801EEC1C; /* @source 0x801EEC1C @kind data */
extern u8 *D_801F20DC; /* @source 0x801F20DC @kind data */
extern u32 D_80143E6C; /* @source 0x80143E6C @kind data */
extern void func_801F00B0(void); /* @source 0x801F00B0 @kind function */
extern void func_801EFBE4(u16 phase, u16 amount); /* @source 0x801EFBE4 @kind function */
extern void func_801EFE18(u16 phase, u16 amount); /* @source 0x801EFE18 @kind function */

/* @source 0x801EF754
 * @behavior Copies three handler pointers from 0x801EEC1C into the frame
 *           and dispatches record byte 2 through the pointer at 0x801F20DC.
 *           If record byte 0 bit 0 and byte 2 are nonzero, calls 0x801F00B0.
 *           If the reloaded record halfword at 0x0C has either low bit set,
 *           calls 0x801EFBE4 and 0x801EFE18 with half the shared counter
 *           (the latter plus four), narrowed to 16 bits, and record byte 6
 *           (the latter multiplied by three halves with truncation).
 *           Takes no arguments and returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void dispatchRecordByte2AndRunFlaggedCallbacksBmagicMagic07403_801EF754(void) {
  BmagicMagic07403FlaggedTable handlers;
  u8 *dispatchRecord;
  u8 *record;

  dispatchRecord = D_801F20DC;
  handlers = D_801EEC1C;
  handlers.handlers[dispatchRecord[2]]();
  record = D_801F20DC;
  if ((record[0] & 1) && record[2] != 0) {
    func_801F00B0();
    record = D_801F20DC;
    if (FIELD_REF(u16, record, 0x0Cu) & 3) {
      func_801EFBE4((u16)(D_80143E6C >> 1), record[6]);
      func_801EFE18((u16)((D_80143E6C >> 1) + 4),
                   D_801F20DC[6] + (D_801F20DC[6] >> 1));
    }
  }
}
