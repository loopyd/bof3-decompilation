#include "bof3/bof3.h"

/* Palette block scaler of this overlay: 0x80 is the unity scale. */
void scalePaletteBlocksWorld04Area19813_801F2D28(s32 scale);

/* Shared work-area reset of the main executable: clears the scratchpad game
 * work-area header fields flags_00, unk_01, flags_02 and pad_03. Its generated
 * undefined-functions map already binds the address, so the body needs no map
 * row for it. */
void func_80196070(void);

/* Work-record pointer cell at scratchpad byte offset 0x44. The original
 * re-reads the cell for every access, so the body names the cell instead of
 * the constant-address accessor, whose single `lui`/`ori` base the compiler
 * keeps live across both calls. */
extern u8* D_1F800044; /* @source 0x1F800044 @kind data */

/* @source 0x801F2CB8
 * @behavior Overlay step handler, entry 2 of the handler run that opens the
 *           reviewed textbin row D_801F3DFC: its first three words are the
 *           code pointers 0x801F2C48, 0x801F2C80 and 0x801F2CB8, and the
 *           bytes after them are table data, so state 2 is this body and no
 *           jal names it. It advances the palette scale carried in byte 0x09
 *           of the work record published at the scratchpad pointer cell
 *           0x1F800044, then republishes both 0x100-entry colour palette
 *           blocks through the payload scaler with that new byte as the
 *           scale, where 0x80 is unity, so the scale walks the blocks up from
 *           the black that entry 0 stores. Once the byte reaches 0x80 it calls
 *           func_80196070, the shared main-executable reset that clears the
 *           scratchpad work-area header, so the run ends in the state it
 *           started from. The cell is re-read for each of the three uses: the
 *           frame setup is scheduled after the first read, and the byte is
 *           loaded again for the scaler argument and again for the bound test
 *           because the increment stores through the pointer; the named
 *           pointer-cell extern keeps that per-read `lui`/`lw` form instead of
 *           the accessor's single live base. The 0x18-byte frame holds only
 *           $ra at 0x10 across the two calls. It takes no arguments and
 *           returns nothing.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateRescaleBlocksUnity(
    void) {
  D_1F800044[9]++;
  scalePaletteBlocksWorld04Area19813_801F2D28(D_1F800044[9]);
  if (D_1F800044[9] >= 0x80) {
    func_80196070();
  }
}
