#include "bof3/battle/battle15_internal.h"

/* State byte run starting at record +0x08: one 0x140-byte party row per
 * battler, indexed by the acting battler byte. The base keeps its own symbol
 * because the original materialises it with a `lui %hi`/`addiu %lo` pair into
 * a register and adds the row offset. */
extern u8 battlePartyStateBytesAt80145FBC[][0x140]; /* @source 0x80145FBC @kind table */

/* Resolved dragon-form outputs: the form code (gene-table row index + 4) and
 * the group byte the gene table holds for that row. */
extern u8 D_800B6F58; /* @source 0x800B6F58 @kind data */
extern u8 D_800B6F5C; /* @source 0x800B6F5C @kind data */

/* Byte cell that takes the group byte of the resolved form. */
extern u8 D_801463B9; /* @source 0x801463B9 @kind data */

extern u8 resolveDragonFormFromGenes(void); /* @source 0x800A6C2C */
extern u8 func_800A6E14(u8 value); /* @source 0x800A6E14 */
extern u8 func_800A7C14(u8 value); /* @source 0x800A7C14 */

/* @source 0x800A68A4
 * @behavior Resets the acting battler's party record state and installs the
 *   dragon form resolved from the gene tables. It zeroes the eight state bytes
 *   at +0x08 of the acting battler's 0x140-stride row above 0x80145FBC, clears
 *   bits 0x8000 and 0x10000 of the word at 0x80145FB4 and bits 0x100, 0x200,
 *   0x800, 0x1000, 0x2000, 0x4000 and 0x40000 of the word at 0x80145FB8 - both
 *   rows indexed by the acting battler byte D_80146374, which every use
 *   re-reads - and refreshes through func_800A9BD8. It then clears the form
 *   bytes 0x800B6F58 and 0x800B6F5C and raises bit 0x2 of the flag word. The
 *   form comes from resolveDragonFormFromGenes: the 0xFF result runs
 *   func_800A6E14 with 0xFF, otherwise func_800A7C14 runs with the form, the
 *   form code (form + 4) is stored at 0x800B6F58, code 8 raises flag 0x10000
 *   and codes 7 and 4 raise flag 0x20000. Every path then shifts the four-byte
 *   rows 1 through 5 of the table at 0x80145500 down from the row above,
 *   refills row 0's bytes 0 through 2 from the active byte list 0x801463C4 up
 *   to its count 0x801463C7 and pads the rest with 0xFF, stores the packed byte
 *   (group << 5) + (code & 0x1F) in row 0's byte 3, publishes the code at
 *   0x801463C9 and the group at 0x801463B9, and refreshes again through
 *   func_800A9BD8.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resolveDragonFormThenInstallRowAndRefresh(void)
{
  /* The acting battler byte keeps its address in a register and re-reads the
   * cell per use, so the reads below run through a pointer whose address the
   * original holds across the calls; the first loop takes the cell through a
   * plain byte view instead, which is the spelling that measured its own
   * register-held base. */
  volatile u8 *sel;
  /* Clear mask of the word being cleared; a local keeps the original's
   * pointer/mask register pair. */
  u32 clearMask;
  /* Resolved gene-table row index, or 0xFF. */
  u8 form;
  /* Active byte count; the original keeps this cell's address in a register
   * and reloads it once per copied byte. */
  u8 *count;
  u8 i;

  i = 0;
  do {
    battlePartyStateBytesAt80145FBC[((u8 *)&D_80146374)[0]][i] = 0;
    i++;
  } while (i < 8);
  sel = &D_80146374;
  clearMask = 0xFFFE7FFF;
  D_80145FB4[sel[0]].unk_00 &= clearMask;
  clearMask = 0xFFFB84FF;
  D_80145FB8[sel[0]].flags_00 &= clearMask;
  func_800A9BD8(sel[0]);
  D_800B6F58 = 0;
  D_800B6F5C = 0;
  D_80145FB8[sel[0]].flags_00 |= 0x2;
  form = resolveDragonFormFromGenes();
  if (form != 0xFF) {
    func_800A7C14(form);
    D_800B6F58 = form + 4;
    if (D_800B6F58 == 8) {
      D_80145FB8[sel[0]].flags_00 |= 0x10000;
    }
    if (D_800B6F58 == 7) {
      D_80145FB8[sel[0]].flags_00 |= 0x20000;
    }
    if (D_800B6F58 == 4) {
      D_80145FB8[sel[0]].flags_00 |= 0x20000;
    }
  } else {
    func_800A6E14(0xFF);
  }
  for (i = 5; i != 0; i--) {
    D_80145500[i][0] = D_80145500[i - 1][0];
    D_80145500[i][1] = D_80145500[i - 1][1];
    D_80145500[i][2] = D_80145500[i - 1][2];
    D_80145500[i][3] = D_80145500[i - 1][3];
  }
  /* Representation notes, measured on this body: indexing the first loop with
   * the volatile cell itself scored 207 instructions and 900 bytes, a pointer
   * local for that loop 218, and the plain byte view above 219, with the count
   * pointer below closing all 226. */
  count = &D_801463C7;
  i = 0;
  for (; i < *count; i++) {
    D_80145500[0][i] = D_801463C4[i];
  }
  for (; i < 3; i++) {
    D_80145500[0][i] = 0xFF;
  }
  D_80145500[0][3] = (D_800B6F5C << 5) + (D_800B6F58 & 0x1F);
  D_801463C9 = D_800B6F58;
  D_801463B9 = D_800B6F5C;
  func_800A9BD8(D_80146374);
}
