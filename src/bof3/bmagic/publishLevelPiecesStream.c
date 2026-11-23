#include "bof3/bof3.h"

/* Local work-record table in main RAM, outside this overlay's image: the
 * 0x140-byte-stride record array the front end and the battle overlays select
 * by a global byte. Splat binds the address through this target's generated
 * undefined_syms_auto.txt, so no target-map row is added here. */
extern u8 D_80145E90[]; /* @source 0x80145E90 @kind table */

/* Battle selection byte in main RAM, outside this overlay's image. It is read
 * unsigned, once, and scaled into the record table above. */
extern u8 D_80146374; /* @source 0x80146374 @kind unknown */

/* Scratchpad work-object pointer cell, already bound by this target's map row
 * g_battle_work = 0x1F800044. Declared as a named symbol rather than through
 * SPAD_PTR_SLOT so every access re-materialises the cell address with its own
 * lui, which is what the original does: it loads the cell into a saved
 * register, stores the selected record back through the cell, restores the
 * saved pointer before the stream call and re-reads the cell to advance the
 * restored work object. */
extern u8 *g_battle_work; /* @source 0x1F800044 @kind data */

/* Shared battle work record pointer, outside this payload. This target's
 * generated undefined-symbol table binds 0x801EB4E0 and the exact same-target
 * lifts incrementWorkByte2WhenGlobalWorkByte2Is10BmagicMagic06403_801F0E50 and
 * stateInvokeHelperNo
 * declare it the same way. The original reads the record's halfword at 0x2C
 * and its byte at 0x8, once each per branch block. */
extern u8 *D_801EB4E0; /* @source 0x801EB4E0 @kind unknown */

/* Sprite-piece drawer at 0x801F0EEC, still an asm row of this target: it builds
 * a sprite primitive from the work object's halfwords 0x2E and 0x30 and writes
 * the level it is passed into that primitive's three colour bytes. The level
 * parameter is a byte in the callee's own use and the original truncates the
 * computed difference with andi 0xFF at this call site, so it is declared u8. */
extern void func_801F0EEC(u32 index, u32 abr, u8 level);

/* @source 0x801EF2A4
 * @behavior Entry of the payload code-pointer run based at 0x801EEC04, whose
 *           word at 0x801EEC30 names this span. Publishes the selected local
 *           work record, re-levels the four sprite pieces that draw it by the
 *           previous work object's byte 9, starts the EMI stream slot the
 *           shared battle record selects, then advances work byte 2. The body
 *           reads the battle selection byte at 0x80146374 unsigned, keeps the
 *           scratchpad work-object pointer cell 0x1F800044 in a saved register,
 *           and stores the address of the 0x140-byte-stride record at that
 *           index of the D_80145E90 table into the cell. With that record
 *           published it calls the sprite-piece drawer func_801F0EEC four
 *           times, once per piece: pieces 1 and 2 take 0x80 minus the saved
 *           object's byte 9 while pieces 0 and 3 take that byte unchanged, and
 *           the constant 0x80 is materialised once into a saved register for
 *           both subtractions. It then reads the shared battle record pointer
 *           at 0x801EB4E0, keeps that record's halfword 0x2C, restores the
 *           saved work object through the pointer cell and starts the EMI
 *           stream slot func_80161FDC with 0x240, 0x242, 0x241 or 0x243: the
 *           halfword being zero selects the 0x240/0x242 pair and a non-zero
 *           value the 0x241/0x243 pair, and within either pair the record's
 *           unsigned byte 8 below 2 selects the even value and 2 or more the
 *           odd one. Finally it re-reads the pointer cell and increments the
 *           restored work object's byte 2 with a byte-width add, the step index
 *           the byte-2 dispatcher reads next. Takes no arguments and returns
 *           nothing; the 0x20-byte frame keeps the return address at 0x18, the
 *           saved pointer at 0x14 and the 0x80 constant at 0x10. The roles of
 *           work byte 9 and of the piece index are not established here, so the
 *           name reports the literal levels this body writes.
 *
 *           Shape evidence: this target's func_801EF3C8 (asm row) publishes the
 *           same selected record and re-levels the same four pieces with the
 *           identical call order, constants and 0x80-minus-byte-9 spelling, and
 *           it was the donor for that block's statement order. The tail from the
 *           shared-record read onward is this span's own bytes.
 * @status exact
 * @match 100.00
 * @residual none
 */
void publishLevelPiecesStream(
    void) {
  u8 *saved;
  u8 *record;
  u8 index;
  u16 state;
  u32 slot;

  index = D_80146374;
  saved = g_battle_work;
  g_battle_work = &D_80145E90[index * 0x140];
  func_801F0EEC(1, 2, 0x80 - saved[9]);
  func_801F0EEC(0, 1, 0x80 - saved[9]);
  func_801F0EEC(3, 2, saved[9]);
  func_801F0EEC(2, 1, saved[9]);
  record = D_801EB4E0;
  state = *(u16 *)(record + 0x2C);
  g_battle_work = saved;
  if (state == 0) {
    if (record[8] < 2) {
      slot = 0x240;
    } else {
      slot = 0x242;
    }
  } else {
    if (record[8] < 2) {
      slot = 0x241;
    } else {
      slot = 0x243;
    }
  }
  func_80161FDC(slot);
  g_battle_work[2]++;
}
