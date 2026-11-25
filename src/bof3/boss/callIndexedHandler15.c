#include "bof3/bof3.h"

typedef void (*BossBoss05116Handler)(void);

/* Handler run this dispatcher selects from, based at 0x800C1DFC, the 0x4C-byte
 * offset inside the data row that the Splat `textbin` row D_800C1DB0 places at
 * the end of this payload. Its three in-payload words are this overlay's inert
 * handlers 0x800C19FC, 0x800C1A04 and 0x800C1A0C, and the payload ends one word
 * past the last entry at 0x800C1E08, so this declaration binds only the word the
 * scaled index is added to and the run stays one row. */
extern BossBoss05116Handler D_800C1DFC[]; /* @source 0x800C1DFC @kind data */

/* @source 0x800C19C4
 * @behavior Byte-indexed overlay dispatcher: masks the index argument with 0xFF,
 * scales it by four and calls the handler selected from the three-word run based
 * at 0x800C1DFC, so the selected handler's result stays in the return register.
 * The original bounds nothing before the scaled table load, the index shift and
 * that table load fill the two slots ahead of the jump, and the slot after the
 * table load stays a nop. It reads no state and writes none; the 0x18-byte frame
 * only keeps $ra at 0x10 across the indirect call. No `jal` in this payload
 * names 0x800C19C4: the only in-payload reference is the word the step-setup
 * publisher at 0x800C1858 materializes as 0x800C19C4 and stores into +0xE4 of
 * the current enemy work record, so whoever calls that stored slot supplies the
 * index and the argument role stays structural. The 56-byte body differs from
 * the sibling dispatchIndexedHandlerTable1e0cBossBoss00716_800C19B4 only in the
 * `%lo` immediate of the table load at +0x18, 0x1DFC here against 0x1E0C there,
 * and every other word equal; dispatchIndexedHandlerBossBoss00716_800C18BC
 * (0x1DD0) and dispatchIndexedHandlerBossBoss05016_800C18B8 (0x1B7C) differ in
 * that same word alone.
 * @status exact
 * @match 100.00
 * @residual none
 */
void callIndexedHandler15(u8 index) {
  D_800C1DFC[index]();
}
